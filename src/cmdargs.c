#include <string.h>

#include <ylang.h>
#include Y_CMDARGS
#include Y_UTIL

#define WS(STRING) fputs(STRING, stream)
#define WC(CHAR) fputc(CHAR, stream)
#define WFMT(FMT, ...) fprintf(stream, FMT,##__VA_ARGS__)

static bool is_file(const char *input);

static void write_doc(FILE *stream, const Y_CmdDoc *doc, unsigned int tabs);
static void write_tabs(FILE *stream, unsigned int tabs);
static void write_multiline(FILE *stream, unsigned int tabs, const char *text);
static void write_doc_name(FILE *stream, const char *name);
static void write_doc_description(FILE *stream, const char *descritpion, unsigned int tabs);
static void write_required_arguments_usage(FILE *stream, const Y_CmdMgr *mgr);
static void write_flags_usage(FILE *stream, const Y_CmdMgr *mgr);
static void write_rest_usage(FILE *stream, const Y_CmdMgr *mgr);
static void write_required_docs(FILE *stream, const Y_CmdMgr *mgr);
static void write_rest_docs(FILE *stream, const Y_CmdMgr *mgr);
static void write_rest_amout(FILE *stream, const Y_CmdRest *rest);
static void write_type_docs(FILE *stream, const Y_CmdInputType *type, unsigned int tabs);
static void write_flags_docs(FILE *stream, const Y_CmdMgr *mgr);
static bool str_overlaps(const char *first, const char *second);

const Y_CmdInputType *y_cmd_type_file = &(Y_CmdInputType){
    .doc = {
        .name = "file",
        .description = "path to the file",
    },
    .is_satisfies = is_file,
};

void y_cmd_mgr_write_docs(FILE *stream, const Y_CmdMgr *mgr){
    if(mgr == NULL){
        Y_FAULT("%s(): mgr == NULL", __func__);
    }

    write_doc(stream, &mgr->doc, 1);
    WC('\n');

    WS("USAGE:\n");
    write_tabs(stream, 1);
        write_doc_name(stream, mgr->doc.name);
        write_required_arguments_usage(stream, mgr);
        write_flags_usage(stream, mgr);
        write_rest_usage(stream, mgr);
    WS("\n\n");

    write_required_docs(stream, mgr);
    WS("\n");

    write_rest_docs(stream, mgr);
    WS("\n");

    write_flags_docs(stream, mgr);

    fflush(stream);
}

bool y_cmd_mgr_flags_overlapped(const Y_CmdMgr *mgr){
    if(mgr == NULL){
        Y_FAULT("%s(): mgr == NULL", __func__);
    }

    for (unsigned i = 0; i < mgr->flags_cnt; i++){
        for (unsigned j = 0; j < i; j++){
            if(str_overlaps(mgr->flags[i].flag, mgr->flags[j].flag)){
                return true;
            }
        }
    }
    return false;
}

//static

static bool is_file(const char *input){
    if(input){
        Y_TODO("is_filepath()");
        return true;
    }
    else{
        return false;
    }
}

static void write_doc(FILE *stream, const Y_CmdDoc *doc, unsigned int tabs){
    if(doc == NULL){
        WS("***undocumented***");
    }
    else{
        write_doc_name(stream, doc->name);
        WS(":\n");

        write_tabs(stream, tabs );
        write_doc_description(stream, doc->description, tabs);
    }
    WC('\n');
}

static void write_tabs(FILE *stream, unsigned int tabs){
    while (tabs--){
        WS("    ");
    }
}

static void write_multiline(FILE *stream, unsigned int tabs, const char *text){
    if(text){
        while (*text){
            WC(*text);
            if(*text == '\n'){
                write_tabs(stream, tabs);
            }
            text++;
        }
    }
}

static void write_doc_name(FILE *stream, const char *name){
    if(name){
        WS(name);
    }
    else{
        WS("***unnamed***");
    }
}

static void write_doc_description(FILE *stream, const char *descritpion, unsigned int tabs){
    if(descritpion){
        write_multiline(stream, tabs, descritpion);
    }
    else{
        WS("***undescribed***");
    }
}

static void write_required_arguments_usage(FILE *stream, const Y_CmdMgr *mgr){
    if(!mgr) return;

    for(unsigned int i = 0; i < mgr->requred_cnt; i++){
        const Y_CmdRequred *required = mgr->requred + i;

        WC(' ');
        WC('<');
        write_doc_name(stream, required->doc.name);

        WC(':');

        if(mgr->requred[i].input_type){
            write_doc_name(stream, required->input_type->doc.name);
        }
        else{
            WS("NULL");
        }
        WC('>');
    }
}

static void write_flags_usage(FILE *stream, const Y_CmdMgr *mgr){
    if(!mgr) return;
    for(unsigned int i = 0; i < mgr->flags_cnt; i++){
        const Y_CmdFlag *flag = mgr->flags + i;

        WC(' ');
        WS("[-");
        if(flag->flag){
            WS(flag->flag);
        }
        else{
            WS("NULL");
        }

        WC(':');

        if(flag->input_type){
            write_doc_name(stream, flag->input_type->doc.name);
        }
        else{
            WS("NULL");
        }
        WC(']');
    }
}

static void write_rest_usage(FILE *stream, const Y_CmdMgr *mgr){
    if(!mgr || mgr->rest) return;
    
    WS(" [");
    write_doc_name(stream, mgr->rest->doc.name);
    WC(':');
    if(mgr->rest->input_type){
        write_doc_name(stream, mgr->rest->input_type->doc.name);
    }
    else{
        WS("NULL");
    }
    WFMT(" ... (>= %u", mgr->rest->min_cout);
    if(mgr->rest->max_cout >= 0){
        WFMT(", < %i", mgr->rest->max_cout + 1);
    }
    WS(" items)");
    WC(']');
}

static void write_required_docs(FILE *stream, const Y_CmdMgr *mgr){
    if(!mgr || !mgr->requred_cnt) return;

    WS("REQUIRED:\n");
    for (unsigned int i = 0; i < mgr->requred_cnt; i++){
        const Y_CmdRequred *required = mgr->requred + i;

        write_tabs(stream, 1);
        write_doc(stream, &required->doc, 2);
        WC('\n');

        write_type_docs(stream, required->input_type, 2);

        if(i != mgr->requred_cnt - 1){
            WS("\n");
        }
    }
}

static void write_rest_docs(FILE *stream, const Y_CmdMgr *mgr){
    if(!mgr || !mgr->rest) return;

    WS("VARIADIC:\n");
    write_tabs(stream, 1);

    const Y_CmdRest *rest = mgr->rest;
    write_doc(stream, &rest->doc, 2);

    WC('\n');
    write_type_docs(stream, rest->input_type, 1);
    WC('\n');

    write_tabs(stream, 1);
    WS("COUNT:\n");
    write_tabs(stream, 2);

    write_rest_amout(stream, rest);
    WS("\n");
}

static void write_rest_amout(FILE *stream, const Y_CmdRest *rest){
    if(rest->min_cout == 0 && rest->max_cout < 0){
        WS("any amount of items");
    }
    else if(rest->min_cout == 0){
        WFMT("less or equal to %i", rest->max_cout);
    }
    else if(rest->max_cout < 0){
        WFMT("greater or equal to %i", (int)rest->min_cout);
    }
    else{
        WFMT("greater or equal to %i and less or equal to %i", (int)rest->min_cout, rest->max_cout);
    }
}

static void write_type_docs(FILE *stream, const Y_CmdInputType *type, unsigned int tabs){
    write_tabs(stream, tabs);
    WS("TYPE:");
    if(type){
        write_doc(stream, &type->doc, tabs + 1);
    }
    else{
        WS("NULL\n");
    }
}

static void write_flags_docs(FILE *stream, const Y_CmdMgr *mgr){
    if(!mgr ||  !mgr->flags_cnt) return;

    WS("FLAGS:\n");

    for (unsigned int i = 0; i < mgr->flags_cnt; i++){
        const Y_CmdFlag *flag = mgr->flags + i;

        write_tabs(stream, 1);
        WFMT("-%s:\n", flag->flag);

        write_tabs(stream, 2);
        write_doc(stream, &flag->doc, 3);
        WS("\n");
        write_type_docs(stream, flag->input_type, 2);

        if(i != mgr->flags_cnt - 1){
            WS("\n");
        }
    }
}

static bool str_overlaps(const char *first, const char *second){
    bool result = false;
    while (*first && *second){
        if(*first++ == *second++){
            result = true;
        }
        else{
            result = false;
        }
    }

    return result;
}
