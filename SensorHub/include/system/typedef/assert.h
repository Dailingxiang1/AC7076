#ifndef __ASSERT_H__
#define __ASSERT_H__

#define ASSERT(a,...)   \
    do { \
        if(!(a)){ \
            printf("file:%s, line:%d", __FILE__, __LINE__); \
            printf("ASSERT-FAILD: "#a" "__VA_ARGS__); \
            while(1); \
        } \
    }while(0);




#endif /* #ifndef _COMMON_H_ */
