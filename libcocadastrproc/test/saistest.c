#include <stdio.h>
#include <string.h>

#include "CuTest.h"

#include "arrays.h"
#include "alphabet.h"
#include "bitbyte.h"
#include "cstrutil.h"
#include "errlog.h"
#include "sais.h"
#include "memdbg.h"
#include "randutil.h"
#include "xstr.h"
#include "xstrformat.h"

void random_str(alphabet *ab, size_t len, char *dest) 
{
    for (size_t i=0; i < len; i++) {
        dest[i] = (char)ab_char(ab, rand_range_size_t(0, ab_size(ab)));
    } 
    dest[len] = '\0';
}

void random_xstr(alphabet *ab, size_t len, xstr *dest) 
{
    xstr_clear(dest);
    for (size_t i = 0; i < len; i++) {
        xstr_push(dest, ab_char(ab, rand_range_size_t(0, ab_size(ab))));
    } 
}


void sais_test_str(CuTest *tc)
{
    memdbg_reset();
    alphabet *ab = alphabet_new(10, "abcdefghij");
    size_t max_len = 1000;
    char *str = cstr_new(max_len);
    for (size_t len = 0; len < max_len; len++) {
        random_str(ab, len, str);
        size_t *sarr = sais(str, len, ab);
        //DEBUG_EXEC(ARR_PRINT(sarr, SA, %zu, 0, len + 1, 10, "   "));    
        CuAssertSizeTEquals(tc, len, sarr[0]);
        for (size_t i = 1; i + 1 < len; i++) {
            CuAssert(tc, "Wrong SARR order", strcmp(&str[sarr[i]], &str[sarr[i + 1]]) < 0);
        }
        FREE(sarr);
    }
    FREE(str);
    alphabet_free(ab);
    CuAssert(tc, "Memory leak", memdbg_is_empty());
}


int xstr_suff_cmp(xstr *str, size_t i, size_t j) 
{
    if (i == j) {
        return 0;
    }
    size_t len = xstr_len(str);
    while (true) {
        if (i == len) {
            return -1;
        }
        else if ( j == len ) {
            return +1;
        }
        else if (xstr_get(str, i) < xstr_get(str, j)) {
            return -1;
        }
        else if (xstr_get(str, i) > xstr_get(str, j)) {
            return +1;
        }
        i++;
        j++;
    }
}

void sais_test_xstr(CuTest *tc)
{
    memdbg_reset();
    alphabet *ab = int_alphabet_new(10);
    size_t max_len = 5000;
    xstr *str = xstr_new(nbytes(ab_size(ab)));
    xstrformat *xf = xstrformat_new(str);
    for (size_t len = 0; len < max_len; len++) {
        random_xstr(ab, len, str);
        CuAssertSizeTEquals(tc, len, xstr_len(str));
        //format_print(xstrformat_as_format(xf));
        size_t *sarr = sais_xstr(str, ab);
        CuAssertSizeTEquals(tc, len, sarr[0]);
        for (size_t i = 1; i + 1 < len; i++) {
            CuAssert(tc, "Wrong SARR order", xstr_suff_cmp(str, sarr[i], sarr[i + 1]) < 0);
        }
        FREE(sarr);
    }
    xstrformat_free(xf);
    xstr_free(str);
    alphabet_free(ab);
    if (!memdbg_is_empty()) {
        DEBUG_EXEC(memdbg_print_stats(stdout, true));
    }
    CuAssert(tc, "Memory leak", memdbg_is_empty());
    
}

void sais_test(CuTest *tc)
{
    memdbg_reset();
    alphabet *ab2 = alphabet_new(10, "0123456789");
    char *str2 = 
"079663176104215914880256659263954945515396273089063732946914831216697995520582879314637650930107706521516417941431178696641176259494707972444072770431631803939852283600825811501989580226849089195328194191409205275913118588899610893171921159127731861964879328726473239436857634501963255475797256201519783660434445852703990257855909513448574413495421288006400030830135421070200294289228079156883015230957538636919975411424651826861592926743879159000713900416978490460197309459295040362720886279344083501576714888201727966622532464965553048017822122673784811166639048511583956866339066512440320922320974830672554097000024998194476763446122634198274636510614186600966523480011329889468206755021749336769958";
    size_t len = strlen(str2);
    printf("str2=%s\n", str2);
    size_t *sarr2 = sais(str2, len, ab2);
    DEBUG_EXEC(ARR_PRINT(sarr2, SA2, %zu, 0, len + 1, 10, "   "));    
    

    alphabet *ab = int_alphabet_new(10);
    xstr *str = xstr_new(nbytes(ab_size(ab)));
    for (size_t i = 0; i< len; i++) {
        xstr_push(str, str2[i]-'0');
    }

    for (size_t i = 0; i<len; i++) {
        CuAssertSizeTEquals(tc, ab_rank(ab, xstr_get(str, i)), ab_rank(ab2, (ullong)(str2[i])));
    }

    xstrformat *xf = xstrformat_new(str);
    format_print(xstrformat_as_format(xf));
    size_t *sarr = sais_xstr(str, ab);
    for (size_t i = 1; i + 1 < len; i++) {
        CuAssert(tc, "Wrong SARR order", xstr_suff_cmp(str, sarr[i], sarr[i + 1]) < 0);
    }
    DEBUG_EXEC(ARR_PRINT(sarr, SA, %zu, 0, len + 1, 10, "   "));    


    xstrformat_free(xf);
    xstr_free(str);
    alphabet_free(ab);

    //CuAssert(tc, "Memory leak", memdbg_is_empty());
    
}

CuSuite *sais_get_test_suite()
{
	CuSuite *suite = CuSuiteNew();
	SUITE_ADD_TEST(suite, sais_test_str);
	SUITE_ADD_TEST(suite, sais_test_xstr);
	//SUITE_ADD_TEST(suite, sais_test);
	return suite;
}
