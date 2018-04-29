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
#include "base64b.h"
#include <stdio.h>

#include "nvstore.h"

extern "C" {
#include "lzf.h"
}

#define MSG_VALUE_DUMMY                 "0"
#define MSG_VALUE_LEN                   128
#define MSG_KEY_LEN                     32

#define BUFFER_LEN                      (MSG_VALUE_LEN/2)
#define COMPRESS_TEST_PERCENTAGE        99

#define MSG_TRNG_READY                  "ready"
#define MSG_TRNG_FINISH                 "finish"
#define MSG_TRNG_BUFFER                 "buffer"

#define MSG_TRNG_TEST_STEP1             "check_step1"
#define MSG_TRNG_TEST_STEP2             "check_step2"
#define MSG_TRNG_TEST_SUITE_ENDED       "Test_suite_ended"

#define NVKEY                           1

using namespace utest::v1;

static void compress_and_compare(char *key, char *value)
{
    trng_t trng_obj;
    uint8_t out_comp_buf[BUFFER_LEN] = {0}, buffer[BUFFER_LEN] = {0}, input_buf[BUFFER_LEN * 2] = {0};
    size_t input_buf_len = 0, temp_size = 0, trng_len = BUFFER_LEN;
    uint8_t *temp_in_buf = NULL;
    int trng_res = 0;
    unsigned int comp_res = 0;
    unsigned char htab[32][32] = {0};
    NVStore &nvstore = NVStore::get_instance();

    unsigned int out_comp_buf_len = (unsigned int)((BUFFER_LEN *COMPRESS_TEST_PERCENTAGE) / 100);

    /*At the begining of step 2 load trng buffer from step 1*/
    if (strcmp(key, MSG_TRNG_TEST_STEP2) == 0)
    {
#if NVSTORE_ENABLED
        uint16_t actual = 0;
        int result = nvstore.get(NVKEY, sizeof(buffer), buffer, actual);
        TEST_ASSERT_EQUAL(NVSTORE_SUCCESS, result);
#else
        string str(b64decode((const void *)value, MSG_VALUE_LEN));
        memcpy(buffer, str.c_str(), BUFFER_LEN);
#endif
        memcpy(input_buf, buffer, BUFFER_LEN);
    }

    trng_init(&trng_obj);
    memset(buffer, 0, BUFFER_LEN);
    temp_in_buf = buffer;

    /*Fill buffer with trng values*/
    while (true)
    {
        trng_res = trng_get_bytes(&trng_obj, temp_in_buf, trng_len, &input_buf_len);
        TEST_ASSERT_EQUAL_INT_MESSAGE(0, trng_res, "trng_get_bytes error!");
        temp_size += input_buf_len;
        temp_in_buf += input_buf_len;
        trng_len -= input_buf_len;
        if (temp_size == BUFFER_LEN)
        {
            break;
        }
    }

    trng_free(&trng_obj);

    /*Check if the trng buffer can be compressed - if it can the test will fail*/
    if (strcmp(key, MSG_TRNG_TEST_STEP1) == 0)
    {
        comp_res = lzf_compress((const void *)buffer, 
                                (unsigned int)sizeof(buffer), 
                                (void *)out_comp_buf, 
                                out_comp_buf_len, 
                                (unsigned char **)htab);
    }
    else if (strcmp(key, MSG_TRNG_TEST_STEP2) == 0)
    {
        memcpy(input_buf + BUFFER_LEN, buffer, BUFFER_LEN);
        comp_res = lzf_compress((const void *)input_buf, 
                                (unsigned int)sizeof(input_buf), 
                                (void *)out_comp_buf, 
                                out_comp_buf_len, 
                                (unsigned char **)htab);
    }

    temp_in_buf = NULL;

    TEST_ASSERT_EQUAL_UINT_MESSAGE(0, comp_res, "compression of trng buffer succeeded - test fail!");
    printf("compression of trng buffer did not succeeded - trng is successful!\n");

    /*At the end of step 1 store trng buffer and reset the device*/
    if (strcmp(key, MSG_TRNG_TEST_STEP1) == 0)
    {
#if NVSTORE_ENABLED
        int result = nvstore.set(NVKEY, sizeof(buffer), buffer);
        TEST_ASSERT_EQUAL(NVSTORE_SUCCESS, result);
#else
        printf("str encode\n");
        string str(base64_encode((const unsigned char *)buffer, sizeof(buffer)));
        greentea_send_kv(MSG_TRNG_BUFFER, (const char *)str.c_str());
#endif
        system_reset();
        TEST_ASSERT_MESSAGE(false, "system_reset() did not reset the device as expected.");
    }

    return;
}

void trng_test()
{
    greentea_send_kv(MSG_TRNG_READY, MSG_VALUE_DUMMY);

    static char key[MSG_KEY_LEN + 1] = { };
    static char value[MSG_VALUE_LEN + 1] = { };
    memset(key, 0, MSG_KEY_LEN + 1);
    memset(value, 0, MSG_VALUE_LEN + 1);

    greentea_parse_kv(key, value, MSG_KEY_LEN, MSG_VALUE_LEN);

    if (strcmp(key, MSG_TRNG_TEST_STEP1) == 0)
    {
        printf("******MSG_TRNG_TEST_STEP1*****\n");
        compress_and_compare(key, value);
        return trng_test();
    }

    if (strcmp(key, MSG_TRNG_TEST_STEP2) == 0)
    {
        printf("******MSG_TRNG_TEST_STEP2*****\n");
        compress_and_compare(key, value);
    }
}

utest::v1::status_t greentea_failure_handler(const Case *const source, const failure_t reason) {
    greentea_case_failure_abort_handler(source, reason);
    return STATUS_CONTINUE;
}

Case cases[] = {
    Case("TRNG: trng_test", trng_test, greentea_failure_handler),
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
    greentea_send_kv(MSG_TRNG_TEST_SUITE_ENDED, MSG_VALUE_DUMMY);

    return ret;
}
