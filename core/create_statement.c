#include "create_statement.h"
#include <stdio.h>
#include <memory.h>
void analyse_create_stm(create_stm_data *cdata){

    printf("%s():called...\n", __FUNCTION__);


}

void destroy_cdata(create_stm_data *cdata){

    memset(cdata, 0, sizeof(*cdata));

}
