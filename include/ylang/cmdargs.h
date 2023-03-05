#ifndef YLANG_CMDARGS_H_
#define YLANG_CMDARGS_H_

#include <stdio.h>
#include <stdbool.h>

#include <ylang.h>
#include Y_DEF

Y_BEG_C

typedef struct Y_CmdInputType Y_CmdInputType;
typedef struct Y_CmdRequred Y_CmdRequred;
typedef struct Y_CmdFlag Y_CmdFlag;
typedef struct Y_CmdRest Y_CmdRest;
typedef struct Y_CmdDoc Y_CmdDoc;
typedef struct Y_CmdMgr Y_CmdMgr;

extern const Y_CmdInputType *y_cmd_type_file;

void y_cmd_mgr_write_docs(FILE *stream, const Y_CmdMgr *mgr);

struct Y_CmdDoc{
    const char *name;
    const char *description;
};

struct Y_CmdInputType{
    bool (*is_satisfies)(const char *input);
    Y_CmdDoc doc;
};

struct Y_CmdFlag{
    void *data;
    void (*on_capture)(void *data, const char *input);

    const char *flag;
    const Y_CmdInputType *input_type;
    Y_CmdDoc doc;
};

struct Y_CmdRequred{
    void *data;
    void (*on_capture)(void *data, const char *input);

    const Y_CmdInputType *input_type;
    Y_CmdDoc doc;
};

struct Y_CmdRest{
    void *data;
    void (*on_capture)(void *data, const char *input);

    unsigned int min_cout;
    int max_cout;
    const Y_CmdInputType *input_type;
    Y_CmdDoc doc;
};

struct Y_CmdMgr{
    unsigned int requred_cnt;
    Y_CmdRequred *requred;

    unsigned int flags_cnt;
    Y_CmdFlag *flags;

    const Y_CmdRest *rest;
    
    Y_CmdDoc doc;
};

Y_END_C
#endif //YLANG_CMDARGS_H_

