/*
* Copyright (c) 2018 ARM Limited. All rights reserved.
* SPDX-License-Identifier: Apache-2.0
* Licensed under the Apache License, Version 2.0 (the License); you may
* not use this file except in compliance with the License.
* You may obtain a copy of the License at
*
* http://www.apache.org/licenses/LICENSE-2.0
*
* Unless required by applicable law or agreed to in writing, software
* distributed under the License is distributed on an AS IS BASIS, WITHOUT
* WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
* See the License for the specific language governing permissions and
* limitations under the License.
*/

#include "greentea-client/test_env.h"
#include "unity/unity.h"
#include "utest/utest.h"
#include "hal/trng_api.h"
#include <stdio.h>

extern "C" {
#include "lzf.h"
}

#define LEN 512
#define COMPRESS_TEST_TH_PERCENTS 99

#define MSG_VALUE_DUMMY "0"
#define MSG_VALUE_LEN 32
#define MSG_KEY_LEN 32

#define MSG_KEY_DEVICE_READY "ready"
#define MSG_KEY_DEVICE_FINISH "finish"
//#define MSG_KEY_TRNG_BUFFER "buffer"

#define MSG_KEY_DEVICE_TEST_STEP1 "check_consistency_step1"
#define MSG_KEY_DEVICE_TEST_STEP2 "check_consistency_step2"
#define MSG_KEY_DEVICE_TEST_SUITE_ENDED "Test suite ended"

//#if defined(DEVICE_TRNG)

using namespace utest::v1;

unsigned char htab[32][32] = {0};
trng_t trng_obj;
unsigned int output_buf_len = (unsigned int)((LEN * COMPRESS_TEST_TH_PERCENTS) / 100);
uint8_t input_buf[LEN * 2] = {0};

static void compress_and_compare(char * step)
{
    uint8_t output_buf[LEN] = {0};
    size_t input_buf_len = 0, temp_size = 0, trng_len = LEN;
    uint8_t * temp_in_buf = input_buf;
    int ret = 0;

    /*FILE *fd = fopen("file", "w+");
    size_t res = fwrite(input_buf, 1, LEN, fd);
    TEST_ASSERT_EQUAL(LEN, res);
    res = fclose(fd);*/

    trng_init(&trng_obj);

    unsigned int compressResult = lzf_compress((const void *)input_buf, 
                                               (unsigned int)trng_len, 
                                               (void *)output_buf, 
                                               output_buf_len, 
                                               (unsigned char **)htab);

    // compressResult equals to 0 means that the compress function wasn't able to fit the compressed pBufIn
    // into pBufOut (which is threshold % of pBufIn), this means that the random function did the job good.
    if (compressResult > 0) {
        printf("1 compression worked - test fail\n");
    }
    else
    {
        printf("1 compression didnt worked - test success\n");
        uint8_t print[10] = {0};
        memcpy(print, input_buf, 10);
        printf("input_buf 10 bytes = %s\n", print);
    }

    printf("compress_and_compare - start\n");
    while (true)
    {
        ret = trng_get_bytes(&trng_obj, temp_in_buf, trng_len, &input_buf_len);
        if (ret > 0) 
        {
            printf("trng_get_bytes error!\n");
            break;
        }
        temp_size += input_buf_len;
        temp_in_buf += input_buf_len;
        trng_len -= input_buf_len;
        printf("input_buf_len = %d\n", input_buf_len);
        if (temp_size == LEN)
        {
            printf("trng_get_bytes finish success!\n");
            break;
        }
    }

    temp_in_buf = NULL;
    trng_free(&trng_obj);

    printf("compress_and_compare - htab\n");
    
    compressResult = lzf_compress((const void *)input_buf, 
                                    (unsigned int)input_buf_len, 
                                    (void *)output_buf, 
                                    output_buf_len, 
                                    (unsigned char **)htab);

    // compressResult equals to 0 means that the compress function wasn't able to fit the compressed pBufIn
    // into pBufOut (which is threshold % of pBufIn), this means that the random function did the job good.
    if (compressResult > 0) {
        printf("compression worked - test fail\n");
    }
    else
    {
        printf("compression didnt worked - test success\n");
    }
    printf("compress_and_compare - end\n");

    TEST_ASSERT_EQUAL(0, ret);

     if (strcmp(step, MSG_KEY_DEVICE_TEST_STEP1) == 0)
     {
         printf("in compress_and_compare - MSG_KEY_DEVICE_TEST_STEP1 - reset!\n");
         //greentea_send_kv(MSG_KEY_TRNG_BUFFER, input_buf);
         system_reset();
         TEST_ASSERT_MESSAGE(false, "system_reset() did not reset the device as expected.");
     }

    return;
}

void generate_derived_key_reset_test()
{
    greentea_send_kv(MSG_KEY_DEVICE_READY, MSG_VALUE_DUMMY);

    static char key[MSG_KEY_LEN + 1] = { };
    static char value[MSG_VALUE_LEN + 1] = { };
    memset(key, 0, MSG_KEY_LEN + 1);
    memset(value, 0, MSG_VALUE_LEN + 1);

    greentea_parse_kv(key, value, MSG_KEY_LEN, MSG_VALUE_LEN);

    if (strcmp(key, MSG_KEY_DEVICE_TEST_STEP1) == 0)
    {
        printf("******MSG_KEY_DEVICE_TEST_STEP1*****\n");
        compress_and_compare(MSG_KEY_DEVICE_TEST_STEP1);
        return generate_derived_key_reset_test();
    }

    if (strcmp(key, MSG_KEY_DEVICE_TEST_STEP2) == 0)
    {
        printf("******MSG_KEY_DEVICE_TEST_STEP2*****\n");
        compress_and_compare(MSG_KEY_DEVICE_TEST_STEP2);
    }

    printf("here4\n");
}

utest::v1::status_t greentea_failure_handler(const Case *const source, const failure_t reason) {
    greentea_case_failure_abort_handler(source, reason);
    return STATUS_CONTINUE;
}

Case cases[] = {
    Case("TRNG: generate_derived_key_reset_test", generate_derived_key_reset_test, greentea_failure_handler),
};

utest::v1::status_t greentea_test_setup(const size_t number_of_cases)
{
    GREENTEA_SETUP(100, "trng_reset");
    return greentea_test_setup_handler(number_of_cases);
}

Specification specification(greentea_test_setup, cases, greentea_test_teardown_handler);

int main()
{
    bool ret = !Harness::run(specification);
    greentea_send_kv(MSG_KEY_DEVICE_TEST_SUITE_ENDED, MSG_VALUE_DUMMY);

    return ret;
}