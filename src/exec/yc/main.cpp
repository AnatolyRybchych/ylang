#include <memory.h>

#include <ylang.h>
#include Y_UTIL
#include Y_CMDARGS

int main(int argc, char const *argv[]){
    Y_CmdMgr mgr;
    memset(&mgr, 0, sizeof(mgr));

    mgr.doc.name = "yc";
    mgr.doc.description = "ylang compiller";

    // Y_CmdRequred required[] = {
    //     (Y_CmdRequred){
    //         .data = NULL,
    //         .on_capture = NULL,
    //         .input_type = y_cmd_type_file,
    //         .doc = {
    //             .name = "required_argument",
    //             .description = "here is required argument"
    //         },
    //     }
    // };

    Y_CmdFlag flags[] = {
        (Y_CmdFlag){   
            .data = NULL,
            .on_capture = NULL,
            .flag = "o",
            .input_type = y_cmd_type_file,
            .doc = {
                .name = "output_file",
                .description = "path to output file"
            },
        }
    };

    Y_CmdRest rest = {
        .data = NULL,
        .on_capture = NULL,
        .min_cout = 1,
        .max_cout = -1,
        .input_type = y_cmd_type_file,
        .doc = {
            .name = "input_file",
            .description = "path to input file",
        },
    };

    // mgr.requred = required;
    // mgr.requred_cnt = Y_ARRLEN(required);
    mgr.flags = flags;
    mgr.flags_cnt = Y_ARRLEN(flags);
    mgr.rest = &rest;
    assert(!y_cmd_mgr_flags_overlapped(&mgr));

    //y_cmd_mgr_write_docs(stdout, &mgr);

    if(y_cmd_mgr_try_cpature(&mgr, argc, argv, stdout, true)){
        return 0;
    }
    else{
        return 1;
    }

    return 0;
}
