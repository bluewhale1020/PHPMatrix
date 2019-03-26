/*
  +----------------------------------------------------------------------+
  | PHP Version 7                                                        |
  +----------------------------------------------------------------------+
  | Copyright (c) 1997-2018 The PHP Group                                |
  +----------------------------------------------------------------------+
  | This source file is subject to version 3.01 of the PHP license,      |
  | that is bundled with this package in the file LICENSE, and is        |
  | available through the world-wide-web at the following url:           |
  | http://www.php.net/license/3_01.txt                                  |
  | If you did not receive a copy of the PHP license and are unable to   |
  | obtain it through the world-wide-web, please send a note to          |
  | license@php.net so we can mail you a copy immediately.               |
  +----------------------------------------------------------------------+
  | Author:                                                              |
  +----------------------------------------------------------------------+
*/
#include <math.h>
/* $Id$ */

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include "php.h"
#include "php_ini.h"
#include "ext/standard/info.h"
#include "php_matrix.h"

/* If you declare any globals in php_matrix.h uncomment this:
ZEND_DECLARE_MODULE_GLOBALS(matrix)
*/

/* True global resources - no need for thread safety here */
static int le_matrix;

/* {{{ PHP_INI
 */
/* Remove comments and fill if you need to have entries in php.ini
PHP_INI_BEGIN()
    STD_PHP_INI_ENTRY("matrix.global_value",      "42", PHP_INI_ALL, OnUpdateLong, global_value, zend_matrix_globals, matrix_globals)
    STD_PHP_INI_ENTRY("matrix.global_string", "foobar", PHP_INI_ALL, OnUpdateString, global_string, zend_matrix_globals, matrix_globals)
PHP_INI_END()
*/
/* }}} */

/* Remove the following function when you have successfully modified config.m4
   so that your module can be compiled into PHP, it exists only for testing
   purposes. */

/* Every user-visible function in PHP should document itself in the source */
/* {{{ proto string confirm_matrix_compiled(string arg)
   Return a string to confirm that the module is compiled in */
PHP_FUNCTION(confirm_matrix_compiled)
{
	char *arg = NULL;
	size_t arg_len, len;
	zend_string *strg;

	if (zend_parse_parameters(ZEND_NUM_ARGS(), "s", &arg, &arg_len) == FAILURE) {
		return;
	}

	strg = strpprintf(0, "Congratulations! You have successfully modified ext/%.78s/config.m4. Module %.78s is now compiled into PHP.", "matrix", arg);

	RETURN_STR(strg);
}
/* }}} */
/* The previous line is meant for vim and emacs, so it can correctly fold and
   unfold functions in source code. See the corresponding marks just before
   function definition, where the functions purpose is also documented. Please
   follow this convention for the convenience of others editing your code.
*/

static inline php_matrix* php_matrix_fetch_object(zend_object* obj) {
    return (php_matrix*)((char *)obj - XtOffsetOf(php_matrix, std));
}

#define Z_MATRIX_OBJ_P(zv) php_matrix_fetch_object(Z_OBJ_P(zv));

static zend_object_handlers matrix_object_handlers;

// 結果の matrix を作成する共通関数
static php_matrix* init_return_value(zval* return_value, long numRows, long numCols) {
    php_matrix* result;

    object_init_ex(return_value, matrix_ce);
    Z_SET_REFCOUNT_P(return_value, 1);
    //ZVAL_MAKE_REF(return_value);

    result = Z_MATRIX_OBJ_P(return_value);
    result->numRows = numRows;
    result->numCols = numCols;
    result->data = ecalloc(result->numRows * result->numCols, sizeof(float));
    return result;
}


// デストラクタ
static void php_matrix_destroy_object(zend_object* object) {
    php_matrix* obj = php_matrix_fetch_object(object);
    zend_objects_destroy_object(object);
}
static void php_matrix_object_free_storage(zend_object* object) {
    php_matrix* obj = php_matrix_fetch_object(object);
    if (obj->data) {
        efree(obj->data);
    }
    zend_object_std_dtor(&obj->std);
}

// コンストラクタ
static zend_object* php_matrix_new(zend_class_entry* ce) {
    php_matrix* obj = (php_matrix*)ecalloc(1, sizeof(php_matrix) + zend_object_properties_size(ce));
    zend_object_std_init(&obj->std, ce);
    object_properties_init(&obj->std, ce);

    obj->std.handlers = &matrix_object_handlers;
    return &obj->std;
}

// __construct() メソッド
PHP_METHOD(Matrix, __construct) {
    long numRows, numCols;
    if (zend_parse_parameters(ZEND_NUM_ARGS(), "ll",
            &numRows, &numCols) == FAILURE) {
        return;
    }
    php_matrix* object = Z_MATRIX_OBJ_P(getThis());
    object->numRows = numRows;
    object->numCols = numCols;
    object->data = ecalloc(numRows * numCols, sizeof(float));
    RETURN_TRUE;
}

// shape() メソッド
PHP_METHOD(Matrix, shape) {
    php_matrix* object = Z_MATRIX_OBJ_P(getThis());

    zval shape;
    array_init_size(&shape, 2);
    add_index_long(&shape, 0, object->numRows);
    add_index_long(&shape, 1, object->numCols);

    RETURN_ZVAL(&shape, 1, 1);
}

// (i, j) 成分を取得
PHP_METHOD(Matrix, get) {
    long i, j;
    if (zend_parse_parameters(ZEND_NUM_ARGS(), "ll", &i, &j) == FAILURE) {
        return;
    }
    php_matrix* object = Z_MATRIX_OBJ_P(getThis());
    RETURN_DOUBLE(object->data[i * object->numCols + j]);
}

// (i, j) 成分を設定
PHP_METHOD(Matrix, set) {
    long i, j;
    double val;
    // if (zend_parse_parameters(ZEND_NUM_ARGS(), "lld", &i, &j, &val) == FAILURE) {
    //     return;
    // }
    /* 引数をパースして a, b に代入 */
    ZEND_PARSE_PARAMETERS_START(3, 3)
        Z_PARAM_LONG(i)
        Z_PARAM_LONG(j)
        Z_PARAM_DOUBLE(val)        
    ZEND_PARSE_PARAMETERS_END();

    // printf("(i, j, val) = (%ld, %ld, %f)\n", i, j, val);
    php_matrix* object = Z_MATRIX_OBJ_P(getThis());
    object->data[i * object->numCols + j] = val;
}

// expandRows($matrix,$num_row)
PHP_METHOD(Matrix,expandRows) {
    php_matrix* self = Z_MATRIX_OBJ_P(getThis());


    if (!self) {
        RETURN_FALSE;
    }
    long newRows;
    /* 引数をパースして a, b に代入 */
    ZEND_PARSE_PARAMETERS_START(1, 1)
        Z_PARAM_LONG(newRows)
    ZEND_PARSE_PARAMETERS_END();

    // long r = self->numRows;
    long c = self->numCols;

    // 結果の matrix を作成
    php_matrix* result = init_return_value(return_value, newRows, c);

    for (long i = 0; i < newRows; ++i) {
        for (long j = 0; j < c; ++j) {
            result->data[i * c + j] = self->data[j];
        }
    }

}

// expandColumns($matrix,$num_column)
PHP_METHOD(Matrix,expandColumns) {
    php_matrix* self = Z_MATRIX_OBJ_P(getThis());


    if (!self) {
        RETURN_FALSE;
    }
    long newCols;
    /* 引数をパースして a, b に代入 */
    ZEND_PARSE_PARAMETERS_START(1, 1)
        Z_PARAM_LONG(newCols)
    ZEND_PARSE_PARAMETERS_END();

    long r = self->numRows;
    // long c = self->numCols;

    // 結果の matrix を作成
    php_matrix* result = init_return_value(return_value, r, newCols);

    for (long i = 0; i < r; ++i) {
        for (long j = 0; j < newCols; ++j) {
            result->data[i * newCols + j] = self->data[i];
        }
    }

}

// 行列積の実装
// (r1, c1): (a の行数, a の列数)
// (r2, c2): (b の行数, b の列数)

static inline void _mul(float* a, float* b, float* c, int r1, int c1, int r2, int c2) {

    for (int i = 0; i < r1; ++i) {
        for (int j = 0; j < c2; ++j) {
            for (int k = 0; k < c1; ++k) {
                c[i * c2 + j] += a[i * c1 + k] * b[k * c2 + j];
            }
        }
    }
}

// 行列の積
PHP_METHOD(Matrix, mul) {
    php_matrix* self = Z_MATRIX_OBJ_P(getThis());

    zval* z_matrix;
    php_matrix* matrix;
    if (zend_parse_parameters(ZEND_NUM_ARGS(), "O", &z_matrix, matrix_ce) == FAILURE) {
        return;
    }

    if (!self) {
        RETURN_FALSE;
    }
    matrix = Z_MATRIX_OBJ_P(z_matrix);

    int r1 = (int)self->numRows;
    int c1 = (int)self->numCols;
    int r2 = (int)matrix->numRows;
    int c2 = (int)matrix->numCols;

    // 結果の matrix を作成
    php_matrix* result = init_return_value(return_value, r1, c2);


    _mul(self->data, matrix->data, result->data, r1, c1, r2, c2);


}

// 成分ごとの積（アダマール積）
PHP_METHOD(Matrix, componentwiseProd) {
    php_matrix* self = Z_MATRIX_OBJ_P(getThis());

    zval* z_matrix;
    php_matrix* matrix;
    if (zend_parse_parameters(ZEND_NUM_ARGS(), "O", &z_matrix, matrix_ce) == FAILURE) {
        return;
    }

    if (!self) {
        RETURN_FALSE;
    }
    matrix = Z_MATRIX_OBJ_P(z_matrix);

    // 結果の matrix を作成
    long r = self->numRows;
    long c = self->numCols;
    php_matrix* result = init_return_value(return_value, r, c);

    for (long i = 0; i < r; ++i) {
        for (long j = 0; j < c; ++j) {
            result->data[i * c + j] = self->data[i * c + j] * matrix->data[i * c + j];
        }
    }
}

// 和
PHP_METHOD(Matrix, plus) {
    php_matrix* self = Z_MATRIX_OBJ_P(getThis());

    zval* z_matrix;
    php_matrix* matrix;
    if (zend_parse_parameters(ZEND_NUM_ARGS(), "O", &z_matrix, matrix_ce) == FAILURE) {
        return;
    }

    matrix = Z_MATRIX_OBJ_P(z_matrix);

    // 結果の matrix を作成
    long r = self->numRows;
    long c = self->numCols;
    php_matrix* result = init_return_value(return_value, r, c);

    if (r == matrix->numRows) {
        for (long i = 0; i < r; ++i) {
            for (long j = 0; j < c; ++j) {
                result->data[i * c + j] = self->data[i * c + j] + matrix->data[i * c + j];
            }
        }
    } else if (matrix->numRows == 1) {
        // ブロードキャスト
        for (long i = 0; i < r; ++i) {
            for (long j = 0; j < c; ++j) {
                result->data[i * c + j] = self->data[i * c + j] + matrix->data[j];
            }
        }
    }
}

// 差
PHP_METHOD(Matrix, minus) {
    php_matrix* self = Z_MATRIX_OBJ_P(getThis());

    zval* z_matrix;
    php_matrix* matrix;
    if (zend_parse_parameters(ZEND_NUM_ARGS(), "O", &z_matrix, matrix_ce) == FAILURE) {
        return;
    }

    matrix = Z_MATRIX_OBJ_P(z_matrix);

    // 結果の matrix を作成
    long r = self->numRows;
    long c = self->numCols;
    php_matrix* result = init_return_value(return_value, r, c);

    if (r == matrix->numRows) {
        for (long i = 0; i < r; ++i) {
            for (long j = 0; j < c; ++j) {
                result->data[i * c + j] = self->data[i * c + j] - matrix->data[i * c + j];
            }
        }
    } else if (matrix->numRows == 1) {
        // ブロードキャスト
        for (long i = 0; i < r; ++i) {
            for (long j = 0; j < c; ++j) {
                result->data[i * c + j] = self->data[i * c + j] - matrix->data[j];
            }
        }
    }
}

//  PHP_METHOD(Matrix, hello){
//    zval *data, *obj;
//    obj = getThis();

//    data = zend_read_property(matrix_ce, obj, "yourname", sizeof("yourname") - 1, 1,NULL);

//    printf("hello, %s\n", data->value.str->val);
//    RETURN_TRUE;
//  }

// PHP_FUNCTION(my_sum)
// {
//     /* 引数の格納先 */
//     zend_long a, b;

//     /* 引数をパースして a, b に代入 */
//     ZEND_PARSE_PARAMETERS_START(2, 2)
//         Z_PARAM_LONG(a)
//         Z_PARAM_LONG(b)
//     ZEND_PARSE_PARAMETERS_END();

//     /* 和を計算 */
//     zend_long c = a + b;

//     /* 結果を return する */
//     RETURN_LONG(c);
// }

// スカラー倍
PHP_METHOD(Matrix, scale) {
    php_matrix* self = Z_MATRIX_OBJ_P(getThis());

    double scalar;
    // if (zend_parse_parameters(ZEND_NUM_ARGS(), "d", &scalar) == FAILURE) {
    //     return;
    // }
    // 引数をパースして格納先に代入
    ZEND_PARSE_PARAMETERS_START(1, 1)
        Z_PARAM_DOUBLE(scalar)
    ZEND_PARSE_PARAMETERS_END();

    if (!self) {
        RETURN_FALSE;
    }

    long r = self->numRows;
    long c = self->numCols;

    // 結果の matrix を作成
    php_matrix* result = init_return_value(return_value, r, c);

    for (long i = 0; i < r; ++i) {
        for (long j = 0; j < c; ++j) {
            result->data[i * c + j] = self->data[i * c + j] * scalar;
        }
    }

}

// 転置
PHP_METHOD(Matrix, transpose) {
    php_matrix* self = Z_MATRIX_OBJ_P(getThis());

    long r = self->numRows;
    long c = self->numCols;

    // 結果の matrix を作成
    php_matrix* result = init_return_value(return_value, c, r);

    for (long i = 0; i < r; ++i) {
        for (long j = 0; j < c; ++j) {
            result->data[j * r + i] = self->data[i * c + j];
        }
    }
}


PHP_METHOD(Matrix, toArray) {
    php_matrix* self = Z_MATRIX_OBJ_P(getThis());

    long r = self->numRows;
    long c = self->numCols;

    zval array;
    array_init_size(&array, r);
    for (long i = 0; i < r; ++i) {
        zval row;
        array_init_size(&row, c);
        for (int j = 0; j < c; ++j) {
            add_index_double(&row, j, self->data[i * c + j]);
        }
        add_index_zval(&array, i, &row);
    }

    RETURN_ZVAL(&array, 1, 1);

}

// static createFromData メソッド
PHP_METHOD(Matrix, createFromData) {

    zval* z_data;
    if (zend_parse_parameters(ZEND_NUM_ARGS(), "a", &z_data, matrix_ce) == FAILURE) {
        return;
    }
    HashTable* rows = HASH_OF(z_data);

    uint numRows = zend_hash_num_elements(rows);//uint or int?

    zval* row = zend_hash_index_find(rows, 0);
    HashTable* first_row = HASH_OF(row);

    uint numCols = zend_hash_num_elements(first_row);

    // 結果の matrix を作成
    php_matrix* result = init_return_value(return_value, numRows, numCols);

    zval* elm;
    for (long i = 0; i < numRows; ++i) {
        row = zend_hash_index_find(rows, i);
        HashTable* row_hash = HASH_OF(row);
        for (long j = 0; j < numCols; ++j) {
            elm = zend_hash_index_find(row_hash, j);
            convert_to_double_ex(elm);
            result->data[i * numCols + j] = Z_DVAL_P(elm);
        }
    }
}

// static zerosLike メソッド
PHP_METHOD(Matrix, zerosLike) {

    zval* z_matrix;
    php_matrix* matrix;
    if (zend_parse_parameters(ZEND_NUM_ARGS(), "O", &z_matrix, matrix_ce) == FAILURE) {
        return;
    }
    matrix = Z_MATRIX_OBJ_P(z_matrix);

    long numRows = matrix->numRows;
    long numCols = matrix->numCols;

    // 結果の matrix を作成
    init_return_value(return_value, numRows, numCols);
    // ↑ ゼロで初期化されるのでこれでよいはず
}

// static onesLike メソッド
PHP_METHOD(Matrix, onesLike) {

    zval* z_matrix;
    php_matrix* matrix;
    if (zend_parse_parameters(ZEND_NUM_ARGS(), "O", &z_matrix, matrix_ce) == FAILURE) {
        return;
    }
    matrix = Z_MATRIX_OBJ_P(z_matrix);

    long numRows = matrix->numRows;
    long numCols = matrix->numCols;

    // 結果の matrix を作成
    php_matrix* result = init_return_value(return_value, numRows, numCols);

    // 1 を代入していく
    for (long i = 0; i < numRows; ++i) {
        for (long j = 0; j < numCols; ++j) {
            result->data[i * numCols + j] = 1.0;
        }
    }
}



// activatefunction relu
PHP_METHOD(Matrix,reluFunc) {
    php_matrix* self = Z_MATRIX_OBJ_P(getThis());


    if (!self) {
        RETURN_FALSE;
    }

    long r = self->numRows;
    long c = self->numCols;

    // 結果の matrix を作成
    php_matrix* result = init_return_value(return_value, r, c);

    for (long i = 0; i < r; ++i) {
        for (long j = 0; j < c; ++j) {
            result->data[i * c + j] = MAMIN(MAMAX(0.002*self->data[i * c + j],self->data[i * c + j]), 6);
        }
    }

}

// activatefunction relu derivative
PHP_METHOD(Matrix,reluDer) {
    php_matrix* self = Z_MATRIX_OBJ_P(getThis());


    if (!self) {
        RETURN_FALSE;
    }

    long r = self->numRows;
    long c = self->numCols;

    // 結果の matrix を作成
    php_matrix* result = init_return_value(return_value, r, c);

    for (long i = 0; i < r; ++i) {
        for (long j = 0; j < c; ++j) {
            result->data[i * c + j] = ((self->data[i * c + j] > 0)? 1:0.002);
        }
    }

}

// activatefunction tanh
PHP_METHOD(Matrix,tanhFunc) {
    php_matrix* self = Z_MATRIX_OBJ_P(getThis());


    if (!self) {
        RETURN_FALSE;
    }

    long r = self->numRows;
    long c = self->numCols;

    // 結果の matrix を作成
    php_matrix* result = init_return_value(return_value, r, c);

    for (long i = 0; i < r; ++i) {
        for (long j = 0; j < c; ++j) {
            result->data[i * c + j] = tanhf(self->data[i * c + j]);
        }
    }

}

// activatefunction tanh derivative
PHP_METHOD(Matrix,tanhDer) {
    php_matrix* self = Z_MATRIX_OBJ_P(getThis());


    if (!self) {
        RETURN_FALSE;
    }

    long r = self->numRows;
    long c = self->numCols;

    // 結果の matrix を作成
    php_matrix* result = init_return_value(return_value, r, c);


    for (long i = 0; i < r; ++i) {
        for (long j = 0; j < c; ++j) {
            result->data[i * c + j] = (1 - self->data[i * c + j]*self->data[i * c + j]);
        }
    }

}


// for adam, adjust learning rate
PHP_METHOD(Matrix,adjustLr) {
    php_matrix* self = Z_MATRIX_OBJ_P(getThis());


    double lr;
    double epsilon;

    if (!self) {
        RETURN_FALSE;
    }

    // 引数をパースして格納先に代入
    ZEND_PARSE_PARAMETERS_START(2, 2)
        Z_PARAM_DOUBLE(lr)
        Z_PARAM_DOUBLE(epsilon)        
    ZEND_PARSE_PARAMETERS_END();


    long r = self->numRows;
    long c = self->numCols;

    // 結果の matrix を作成
    php_matrix* result = init_return_value(return_value, r, c);

//$this->lr/(sqrt($value) + $this->betaParams['ϵ']);
    for (long i = 0; i < r; ++i) {
        for (long j = 0; j < c; ++j) {
            result->data[i * c + j] = lr /(sqrtf(self->data[i * c + j]) + epsilon);
        }
    }

}



const zend_function_entry matrix_methods[] = {
	// PHP_ME(Matrix,hello,NULL,ZEND_ACC_PUBLIC)
    PHP_ME(Matrix, __construct, NULL, ZEND_ACC_CTOR | ZEND_ACC_PUBLIC)	
    PHP_ME(Matrix, shape, NULL, ZEND_ACC_PUBLIC)		
    PHP_ME(Matrix, get, NULL, ZEND_ACC_PUBLIC)
    PHP_ME(Matrix, set, NULL, ZEND_ACC_PUBLIC)
    PHP_ME(Matrix, expandRows, NULL, ZEND_ACC_PUBLIC)
    PHP_ME(Matrix, expandColumns, NULL, ZEND_ACC_PUBLIC)   
    PHP_ME(Matrix, mul, NULL, ZEND_ACC_PUBLIC)
    PHP_ME(Matrix, componentwiseProd, NULL, ZEND_ACC_PUBLIC)
    PHP_ME(Matrix, plus, NULL, ZEND_ACC_PUBLIC)
    PHP_ME(Matrix, minus, NULL, ZEND_ACC_PUBLIC)
    PHP_ME(Matrix, scale, NULL, ZEND_ACC_PUBLIC)
    PHP_ME(Matrix, transpose, NULL, ZEND_ACC_PUBLIC)
    PHP_ME(Matrix, toArray, NULL, ZEND_ACC_PUBLIC)
    PHP_ME(Matrix, reluFunc, NULL,  ZEND_ACC_PUBLIC)    
    PHP_ME(Matrix, tanhFunc, NULL,  ZEND_ACC_PUBLIC)    
    PHP_ME(Matrix, reluDer, NULL,  ZEND_ACC_PUBLIC)    
    PHP_ME(Matrix, tanhDer, NULL,  ZEND_ACC_PUBLIC)    
    PHP_ME(Matrix, adjustLr, NULL,  ZEND_ACC_PUBLIC)      
    PHP_ME(Matrix, createFromData, NULL, ZEND_ACC_STATIC | ZEND_ACC_PUBLIC)
    PHP_ME(Matrix, zerosLike, NULL, ZEND_ACC_STATIC | ZEND_ACC_PUBLIC)
    PHP_ME(Matrix, onesLike, NULL, ZEND_ACC_STATIC | ZEND_ACC_PUBLIC)    
    PHP_FE_END  /* Must be the last line in myext_functions[] */
};

/* {{{ php_matrix_init_globals
 */
/* Uncomment this function if you have INI entries
static void php_matrix_init_globals(zend_matrix_globals *matrix_globals)
{
	matrix_globals->global_value = 0;
	matrix_globals->global_string = NULL;
}
*/
/* }}} */

/* {{{ PHP_MINIT_FUNCTION
 */
PHP_MINIT_FUNCTION(matrix)
{
	zend_class_entry ce;
	INIT_CLASS_ENTRY(ce, "Matrix", matrix_methods);
	matrix_ce = zend_register_internal_class(&ce TSRMLS_CC);	
    matrix_ce->create_object = php_matrix_new;
		
    memcpy(&matrix_object_handlers, zend_get_std_object_handlers(), sizeof(zend_object_handlers));
    matrix_object_handlers.offset    = XtOffsetOf(php_matrix, std);
    matrix_object_handlers.clone_obj = NULL;
    matrix_object_handlers.free_obj  = php_matrix_object_free_storage;
    matrix_object_handlers.dtor_obj  = php_matrix_destroy_object;		
	// zend_declare_property_string(matrix_ce,"yourname", strlen("yourname"),"noname", ZEND_ACC_PUBLIC);	
	
	/* If you have INI entries, uncomment these lines
	REGISTER_INI_ENTRIES();
	*/
	return SUCCESS;
}
/* }}} */

/* {{{ PHP_MSHUTDOWN_FUNCTION
 */
PHP_MSHUTDOWN_FUNCTION(matrix)
{
	/* uncomment this line if you have INI entries
	UNREGISTER_INI_ENTRIES();
	*/
	return SUCCESS;
}
/* }}} */

/* Remove if there's nothing to do at request start */
/* {{{ PHP_RINIT_FUNCTION
 */
PHP_RINIT_FUNCTION(matrix)
{
#if defined(COMPILE_DL_MATRIX) && defined(ZTS)
	ZEND_TSRMLS_CACHE_UPDATE();
#endif
	return SUCCESS;
}
/* }}} */

/* Remove if there's nothing to do at request end */
/* {{{ PHP_RSHUTDOWN_FUNCTION
 */
PHP_RSHUTDOWN_FUNCTION(matrix)
{
	return SUCCESS;
}
/* }}} */

/* {{{ PHP_MINFO_FUNCTION
 */
PHP_MINFO_FUNCTION(matrix)
{
	php_info_print_table_start();
	php_info_print_table_header(2, "matrix support", "enabled");
	php_info_print_table_end();

	/* Remove comments if you have entries in php.ini
	DISPLAY_INI_ENTRIES();
	*/
}
/* }}} */

/* {{{ matrix_functions[]
 *
 * Every user visible function must have an entry in matrix_functions[].
 */
const zend_function_entry matrix_functions[] = {
	PHP_FE(confirm_matrix_compiled,	NULL)		/* For testing, remove later. */
	PHP_FE_END	/* Must be the last line in matrix_functions[] */
};
/* }}} */



/* {{{ matrix_module_entry
 */
zend_module_entry matrix_module_entry = {
	STANDARD_MODULE_HEADER,
	"matrix",
	matrix_functions,
	PHP_MINIT(matrix),
	PHP_MSHUTDOWN(matrix),
	PHP_RINIT(matrix),		/* Replace with NULL if there's nothing to do at request start */
	PHP_RSHUTDOWN(matrix),	/* Replace with NULL if there's nothing to do at request end */
	PHP_MINFO(matrix),
	PHP_MATRIX_VERSION,
	STANDARD_MODULE_PROPERTIES
};
/* }}} */

#ifdef COMPILE_DL_MATRIX
#ifdef ZTS
ZEND_TSRMLS_CACHE_DEFINE()
#endif
ZEND_GET_MODULE(matrix)
#endif

/*
 * Local variables:
 * tab-width: 4
 * c-basic-offset: 4
 * End:
 * vim600: noet sw=4 ts=4 fdm=marker
 * vim<600: noet sw=4 ts=4
 */
