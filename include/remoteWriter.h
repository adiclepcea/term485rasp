#include "observer.h"

void processData(struct subscriber *sub);
void initRemoteWriter(const char* server, const char *clientid);
void startCurl(void);
void destroyRemoteWriter(void);
