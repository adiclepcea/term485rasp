package main

import (
	"archive/zip"
	"bytes"
	"crypto/tls"
	"crypto/x509"
	"encoding/json"
	"fmt"
	"io"
	"io/ioutil"
	"log"
	"mime/multipart"
	"net/http"
	"os"
	"strings"
	"sync"
	"time"
)

type configStruct struct {
	ClientID     string `json:"clientid"`
	HeartBeatURL string `json:"heartbeat_server"`
	UploadURL    string `json:"upload_server"`
}

//ReadingSender is used to read the coommands from the server
type ReadingSender struct {
	ID         string      `json:"guid"`
	Name       string      `json:"name"`
	Parameters interface{} `json:"parameters"`
	//Strict will be used to signify that only exact results are accepted
	//like a transaction. For example if we need to send a list of files and
	//one of them is missing, then none will be sent
	Strict bool `json:"strict"`
}

//HeartBeatClientMsg stores the heart beat message from the client
type HeartBeatClientMsg struct {
	ClientID    string `json:"client_id"`
	MessageType string `json:"message_type"`
	Message     string `json:"message"`
}

//FileDownload is a structure used for downloading files
type FileDownload struct {
	Src            string `json:"source"`
	Dest           string `json:"destination"`
	RestartService string `json:"restartservice"`
}

func getSecureHTTPClient() (*http.Client, error) {
	cert, err := tls.LoadX509KeyPair("cert.pem", "key.pem")
	if err != nil {
		return nil, err
	}

	caCert, err := ioutil.ReadFile("ca-chain.cert.pem")
	if err != nil {
		return nil, err
	}

	caCertPool := x509.NewCertPool()
	caCertPool.AppendCertsFromPEM(caCert)

	tlsConfig := &tls.Config{
		Certificates: []tls.Certificate{cert},
		RootCAs:      caCertPool,
	}
	tlsConfig.BuildNameToCertificate()
	transport := &http.Transport{TLSClientConfig: tlsConfig}
	client := &http.Client{Timeout: 5*time.Second, Transport: transport}
	return client, err
}

func archiveFilesByName(archiveName string, files []string) ([]string, []string, error) {
	var missing []string
	var added []string
	//remove the archive file if it still there
	if _, err := os.Stat(archiveName); err == nil {
		err = os.Remove(archiveName)
		if err != nil {
			return nil, nil, err
		}
	}

	zipfile, err := os.Create(archiveName)

	if err != nil {
		return nil, nil, err
	}

	defer zipfile.Close()

	archive := zip.NewWriter(zipfile)

	defer archive.Close()

	for _, file := range files {
		info, err := os.Stat(file)
		if err != nil {
			missing = append(missing, file)
			continue
		}
		header, err := zip.FileInfoHeader(info)
		if err != nil {
			missing = append(missing, file)
			continue
		}

		header.Name = file
		header.Method = zip.Deflate

		writer, err := archive.CreateHeader(header)
		if err != nil {
			missing = append(missing, file)
			continue
		}

		openedFile, err := os.OpenFile(file, os.O_RDONLY, 444)
		if err != nil {
			missing = append(missing, file)
			continue
		}

		defer openedFile.Close()

		_, err = io.Copy(writer, openedFile)

		if err != nil {
			missing = append(missing, file)
			continue
		}

		added = append(added, file)
	}

	return added, missing, nil
}

func postFile(filename string, targetURL string) error {
	bodyBuf := &bytes.Buffer{}
	bodyWriter := multipart.NewWriter(bodyBuf)

	// this step is very important
	fileWriter, err := bodyWriter.CreateFormFile("uploadfile", filename)
	if err != nil {
		fmt.Println("error writing to buffer")
		return err
	}

	client, err := getSecureHTTPClient()

	if err != nil {
		return err
	}

	// open file handle
	fh, err := os.Open(filename)
	if err != nil {
		fmt.Println("error opening file")
		return err
	}

	//iocopy
	_, err = io.Copy(fileWriter, fh)
	if err != nil {
		return err
	}

	contentType := bodyWriter.FormDataContentType()
	bodyWriter.Close()

	resp, err := client.Post(targetURL, contentType, bodyBuf)
	if err != nil {
		return err
	}
	defer resp.Body.Close()
	respBody, err := ioutil.ReadAll(resp.Body)
	if err != nil {
		return err
	}
	fmt.Println(resp.Status)
	fmt.Println(string(respBody))
	return nil
}

//SendHeartBeat sends heartbeat to the server
func SendHeartBeat(config configStruct) error {
	client, err := getSecureHTTPClient()

	if err != nil {
		return err
	}

	hbm := HeartBeatClientMsg{
		ClientID:    config.ClientID,
		MessageType: "HeartBeat",
		Message:     "",
	}
	b := new(bytes.Buffer)
	err = json.NewEncoder(b).Encode(hbm)

	if err != nil {
		return err
	}

	resp, err := client.Post(config.HeartBeatURL, "application/json", strings.NewReader(string(b.Bytes())))

	if err != nil {
		return err
	}
	defer resp.Body.Close()
	
	//TODO check for possible error response code
	
	rs := ReadingSender{}
	decoder := json.NewDecoder(resp.Body)

	err = decoder.Decode(&rs)

	if err != nil {
		return err
	}

	if rs.Name == "FILES REQUEST" {
		return processFilesRequest(rs, config)
	} else if rs.Name == "FILE DOWNLOAD" {
		return processFileDownload(rs)
	}

	//log.Println(string(body))

	return nil

}

func processFileDownload(rs ReadingSender) error {

	mapFD := rs.Parameters.(map[string]interface{})
	buff := bytes.Buffer{}
	err := json.NewEncoder(&buff).Encode(mapFD)
	if err != nil {
		return err
	}

	decoder := json.NewDecoder(strings.NewReader(string(buff.Bytes())))

	fd := FileDownload{}

	err = decoder.Decode(&fd)

	if err != nil {
		return err
	}

	err = downloadFile(fd.Src, fd.Dest)

	if err != nil {
		return err
	}

	if fd.RestartService != "" {
		return fmt.Errorf("%s", "Not implemented yet")
	}

	return nil
}

func processFilesRequest(rs ReadingSender, config configStruct) error {
	var files []string
	for _, iface := range rs.Parameters.([]interface{}) {
		files = append(files, iface.(string))
	}
	fileName := rs.ID + ".zip"
	//fileName = "./archive.zip"
	added, missing, err := archiveFilesByName(fileName, files)

	if err != nil {
		log.Println(err)
		return err
	}

	if len(missing) > 0 && rs.Strict {
		os.Remove(fileName)
		//TODO return an answer stating the found and not found files
		return nil
	}

	log.Println(added)
	log.Println(missing)
	fmt.Println("sending " + fileName)
	postFile(fileName, config.UploadURL)
	os.Remove(fileName)
	return nil

}

func downloadFile(urlsrc string, dest string) error {
	file, err := os.Create(dest)

	if err != nil {
		return err
	}

	defer file.Close()

	client, err := getSecureHTTPClient()

	if err != nil {
		return err
	}

	resp, err := client.Get(urlsrc)

	if err != nil {
		return err
	}

	defer resp.Body.Close()

	log.Println(resp.Status)

	size, err := io.Copy(file, resp.Body)
	if err != nil {
		return err
	}

	log.Printf("Got %d bytes saved in %s\n", size, dest)

	return nil
}

func loadConfig() (*configStruct, error) {
	conf, err := os.Open("config.json")
	if err != nil {
		return nil, err
	}
	confDecoder := json.NewDecoder(conf)

	config := configStruct{}
	err = confDecoder.Decode(&config)
	if err != nil {
		return nil, err
	}

	return &config, nil
}

func main() {
	log.Println("Starting heart beat and configure program ...")

	var w sync.WaitGroup
	w.Add(1)

	quit := make(chan bool)

	config, err := loadConfig()
	if err != nil {
		log.Fatal(err.Error())
		return
	}
	go func() {
		for {
			SendHeartBeat(*config)
			select {
			case <-time.After(5 * time.Second):
			case <-quit:
				w.Done()
				return
			}
		}
	}()
	w.Wait()
}
