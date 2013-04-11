#ifndef DEBUG_H
#define DEBUG_H

#ifdef DEBUG 
#define debug(format,...) printf("File: "__FILE__", Line: %05d: "format"\n", __LINE__, ##__VA_ARGS__)  
#else  
#define debug(format,...)  
#endif  

#endif /* DEBUG_H */

