/*
 +----------------------------------------------------------------------+
 | PHP-OpenCV                                                           |
 +----------------------------------------------------------------------+
 | This source file is subject to version 2.0 of the Apache license,    |
 | that is bundled with this package in the file LICENSE, and is        |
 | available through the world-wide-web at the following url:           |
 | http://www.apache.org/licenses/LICENSE-2.0.html                      |
 | If you did not receive a copy of the Apache2.0 license and are unable|
 | to obtain it through the world-wide-web, please send a note to       |
 | hihozhou@gmail.com so we can mail you a copy immediately.            |
 +----------------------------------------------------------------------+
 | Author: HaiHao Zhou  <hihozhou@gmail.com>                            |
 +----------------------------------------------------------------------+
 */


#include "../../php_opencv.h"
#include "opencv_highgui.h"
#include "core/opencv_mat.h"


#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

/**
 * //todo only cli can call this function
 * CV\imshow
 * @param execute_data
 * @param return_value
 */
PHP_FUNCTION(opencv_imshow){
    char *window_name;
    long window_name_len;
    zval *object;

    if (zend_parse_parameters(ZEND_NUM_ARGS(), "sO", &window_name, &window_name_len, &object,opencv_mat_ce) == FAILURE) {
        RETURN_NULL();
    }
    opencv_mat_object *obj = Z_PHP_MAT_OBJ_P(object);
//    namedWindow(window_name);
    imshow(window_name,*(obj->mat));
    RETURN_NULL();
}


/**
 * CV\waitKey
 * @param execute_data
 * @param return_value
 */
PHP_FUNCTION(opencv_wait_key){
    long delay = 0;

    if (zend_parse_parameters(ZEND_NUM_ARGS(), "|l", &delay) == FAILURE) {
        RETURN_NULL();
    }
    waitKey((int)(delay*1000));//seconds
    RETURN_NULL();
}

/**
 * php namedWindow flags to c++ namedWindow flags
 * @param flags
 * @return
 */
int php_window_flags_to_c_flags(int flags){
    if(flags == 1){
        return WINDOW_NORMAL;
    }else if(flags==2){
        return WINDOW_AUTOSIZE;
    }else if(flags==3){
        return WINDOW_OPENGL;
    }else if(flags==4){
        return WINDOW_FULLSCREEN;
    }else if(flags==5){
        return WINDOW_FREERATIO;
    }else if(flags==6){
        return WINDOW_GUI_EXPANDED;
    }else if(flags==7){
        return WINDOW_GUI_NORMAL;
    }
    return WINDOW_AUTOSIZE;
}

/**
 * CV\namedWindow
 * @param window_name
 * @param flags
 */
PHP_FUNCTION(opencv_named_window){
    char *window_name;
    long window_name_len;
    long flags = 2;

    if (zend_parse_parameters(ZEND_NUM_ARGS(), "s|l", &window_name, &window_name_len, &flags) == FAILURE) {
        RETURN_NULL();
    }
    namedWindow(window_name,php_window_flags_to_c_flags((int)flags));
    RETURN_NULL();
}

struct opencv_fcall_info_struct{
    zend_fcall_info *fci;
    zend_fcall_info_cache *fci_cache;
};

void opencv_create_trackbar_callback(int pos, void* userdata){
    opencv_fcall_info_struct *fci_s=(opencv_fcall_info_struct*)userdata;

    zval retval;
    zval args[1];
    ZVAL_LONG(&args[0], (long)pos);
    fci_s->fci->param_count = 1;
    fci_s->fci->params = args;
    fci_s->fci->retval = &retval;

    //todo zend_call_function: Assertion `((zend_object*)func->op_array.prototype)->gc.u.v.type == 8' failed.
    zend_call_function(fci_s->fci, fci_s->fci_cache);
    zval_ptr_dtor(&args[0]);
}


/**
 * //todo c++ createTrackbar 跳转事件调用php传入的闭包:1php全局变量，2c++闭包
 * CV\createTrackbar
 * @param execute_data
 * @param return_value
 */
PHP_FUNCTION(opencv_create_trackbar){
    char *trackbarname, *winname;
    long value, count, trackbarname_len,winname_len;
    zval retval;

    zend_fcall_info *fci = new zend_fcall_info;
    zend_fcall_info_cache *fci_cache = new zend_fcall_info_cache;
    if (zend_parse_parameters(ZEND_NUM_ARGS(), "ssll|f",
                              &trackbarname, &trackbarname_len,
                              &winname,&winname_len,
                              &value,&count,
                              fci, fci_cache) == FAILURE) {
        return;
    }

    int *trackbar_value_ptr = new int(value);

    opencv_fcall_info_struct *fci_s = new opencv_fcall_info_struct;
    fci_s->fci=fci;
    fci_s->fci_cache = fci_cache;
    createTrackbar(trackbarname, winname, trackbar_value_ptr, (int)count,opencv_create_trackbar_callback,fci_s);
    opencv_create_trackbar_callback(*trackbar_value_ptr,fci_s);
//    createTrackbar(trackbarname, winname, trackbar_value_ptr, (int)count);
//    zend_call_function(fci_s->fci, fci_s->fci_cache);
    RETURN_NULL();

//    if (zend_call_function(fci, fci_cache) == SUCCESS && Z_TYPE(retval) != IS_UNDEF) {
//        zval_ptr_dtor(&args[0]);
//        RETURN_ZVAL(&retval,1,1);
//        ZVAL_COPY_VALUE(return_value, &retval);
//    } else {
//        RETURN_FALSE;
//    }

}



void opencv_highgui_init(int module_number)
{
    /**
     * ! ! Flags for CV\namedWindow
     */
    REGISTER_NS_LONG_CONSTANT(OPENCV_NS, "WINDOW_NORMAL", 1, CONST_CS | CONST_PERSISTENT);
    REGISTER_NS_LONG_CONSTANT(OPENCV_NS, "WINDOW_AUTOSIZE", 2, CONST_CS | CONST_PERSISTENT);
    REGISTER_NS_LONG_CONSTANT(OPENCV_NS, "WINDOW_OPENGL", 3, CONST_CS | CONST_PERSISTENT);
    REGISTER_NS_LONG_CONSTANT(OPENCV_NS, "WINDOW_FULLSCREEN", 4, CONST_CS | CONST_PERSISTENT);
    REGISTER_NS_LONG_CONSTANT(OPENCV_NS, "WINDOW_FREERATIO", 5, CONST_CS | CONST_PERSISTENT);
    REGISTER_NS_LONG_CONSTANT(OPENCV_NS, "WINDOW_GUI_EXPANDED", 6, CONST_CS | CONST_PERSISTENT);
    REGISTER_NS_LONG_CONSTANT(OPENCV_NS, "WINDOW_GUI_NORMAL", 7, CONST_CS | CONST_PERSISTENT);
}