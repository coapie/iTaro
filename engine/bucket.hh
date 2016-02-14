
#ifndef __BUCKET_HH__
#define __BUCKET_HH__

struct ValObject {
    int type:4;
    int size:28;

    char val[0];
};

struct KeyObject {
    int size;
    char key[0];
};

class Bucket {
typedef int64_t TransType;
public:
    // transaction operations
    TransType StartTransaction(); 
    int CommitTransaction(TransType tid);
    int DropTransaction(TransType tid);

    // key - value operations
    int SetKey(KeyObject *ko, ValObject *vo);
    ValObject GetKey(KeyObject *ko); 
};



#endif // __BUCKET_HH__

