
/*
 * C Unit Test Framework
 *
 * Copyright (C) 2003 Asim Jalis
 *
 * https://github.com/ennorehling/cutest
 *
 */


// CuTest's own memory is not tracked: tests call memdbg_reset(), which
// would forget the suites and tests allocated before (see memdbg.h)
#define MEM_DEBUG_OFF

#include <assert.h>
#include <setjmp.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <math.h>

#include "CuTest.h"
#include "memdbg.h"

/*-------------------------------------------------------------------------*
 * CuStr
 *-------------------------------------------------------------------------*/

char *CuStrAlloc(int size)
{
	char *newStr = (char *) malloc( sizeof(char) * (size) );
	return newStr;
}

char *CuStrCopy(const char *old)
{
	int len = strlen(old);
	char *newStr = CuStrAlloc(len + 1);
	strcpy(newStr, old);
	return newStr;
}

/*-------------------------------------------------------------------------*
 * CuString
 *-------------------------------------------------------------------------*/

void CuStringInit(CuString *str)
{
	str->length = 0;
	str->size = STRING_MAX;
	str->buffer = (char *) malloc(sizeof(char) * str->size);
	str->buffer[0] = '\0';
}

CuString *CuStringNew(void)
{
	CuString *str = (CuString *) malloc(sizeof(CuString));
	str->length = 0;
	str->size = STRING_MAX;
	str->buffer = (char *) malloc(sizeof(char) * str->size);
	str->buffer[0] = '\0';
	return str;
}

void CuStringDelete(CuString *str)
{
	if (!str) return;
	free(str->buffer);
	free(str);
}

void CuStringResize(CuString *str, int newSize)
{
	str->buffer = (char *) realloc(str->buffer, sizeof(char) * newSize);
	str->size = newSize;
}

void CuStringAppend(CuString *str, const char *text)
{
	int length;

	if (text == NULL) {
		text = "NULL";
	}

	length = strlen(text);
	if (str->length + length + 1 >= str->size)
		CuStringResize(str, str->length + length + 1 + STRING_INC);
	str->length += length;
	strcat(str->buffer, text);
}

void CuStringAppendChar(CuString *str, char ch)
{
	char text[2];
	text[0] = ch;
	text[1] = '\0';
	CuStringAppend(str, text);
}

void CuStringAppendFormat(CuString *str, const char *format, ...)
{
	va_list argp;
	char buf[HUGE_STRING_LEN];
	va_start(argp, format);
	vsprintf(buf, format, argp);
	va_end(argp);
	CuStringAppend(str, buf);
}

void CuStringInsert(CuString *str, const char *text, int pos)
{
	int length = strlen(text);
	if (pos > str->length)
		pos = str->length;
	if (str->length + length + 1 >= str->size)
		CuStringResize(str, str->length + length + 1 + STRING_INC);
	memmove(str->buffer + pos + length, str->buffer + pos, (str->length - pos) + 1);
	str->length += length;
	memcpy(str->buffer + pos, text, length);
}

/*-------------------------------------------------------------------------*
 * CuTest
 *-------------------------------------------------------------------------*/

void CuTestInit(CuTest *t, const char *name, TestFunction function)
{
	t->name = CuStrCopy(name);
	t->failed = 0;
	t->ran = 0;
	t->message = NULL;
	t->function = function;
	t->jumpBuf = NULL;
}

CuTest *CuTestNew(const char *name, TestFunction function)
{
	CuTest *tc = CU_ALLOC(CuTest);
	CuTestInit(tc, name, function);
	return tc;
}

void CuTestDelete(CuTest *t)
{
	if (!t) return;
	free(t->name);
	free(t);
}

void CuTestRun(CuTest *tc)
{
	printf("Running test %s...\n", (tc->name)?tc->name:"Undefined");
	jmp_buf buf;
	tc->jumpBuf = &buf;
	if (setjmp(buf) == 0) {
		tc->ran = 1;
		(tc->function)(tc);
	}
	tc->jumpBuf = 0;
	printf("Done running %s.\n\n", (tc->name)?tc->name:"Undefined");
}

static void CuFailInternal(CuTest *tc, const char *file, int line,
                           CuString *string)
{
	char buf[HUGE_STRING_LEN];

	sprintf(buf, "%s:%d: ", file, line);
	CuStringInsert(string, buf, 0);

	tc->failed = 1;
	tc->message = string->buffer;
	if (tc->jumpBuf != 0) longjmp(*(tc->jumpBuf), 0);
}

void CuFail_Line(CuTest *tc, const char *file, int line, const char *message2,
                 const char *message)
{
	CuString string;

	CuStringInit(&string);
	if (message2 != NULL) {
		CuStringAppend(&string, message2);
		CuStringAppend(&string, ": ");
	}
	CuStringAppend(&string, message);
	CuFailInternal(tc, file, line, &string);
}

void CuAssert_Line(CuTest *tc, const char *file, int line, const char *message,
                   int condition)
{
	if (condition) return;
	CuFail_Line(tc, file, line, NULL, message);
}

void CuAssertStrEquals_LineMsg(CuTest *tc, const char *file, int line,
                               const char *message,
                               const char *expected, const char *actual)
{
	CuString string;
	if ((expected == NULL && actual == NULL) ||
	        (expected != NULL && actual != NULL &&
	         strcmp(expected, actual) == 0)) {
		return;
	}

	CuStringInit(&string);
	if (message != NULL) {
		CuStringAppend(&string, message);
		CuStringAppend(&string, ": ");
	}
	CuStringAppend(&string, "expected <");
	CuStringAppend(&string, expected);
	CuStringAppend(&string, "> but was <");
	CuStringAppend(&string, actual);
	CuStringAppend(&string, ">");
	CuFailInternal(tc, file, line, &string);
}

void CuAssertCharEquals_LineMsg(CuTest *tc, const char *file, int line,
                                const char *message,
                                char expected, char actual)
{
	char buf[STRING_MAX];
	if (expected == actual) return;
	sprintf(buf, "expected <%c> but was <%c>", expected, actual);
	CuFail_Line(tc, file, line, message, buf);
}


void CuAssertIntEquals_LineMsg(CuTest *tc, const char *file, int line,
                               const char *message,
                               int expected, int actual)
{
	char buf[STRING_MAX];
	if (expected == actual) return;
	sprintf(buf, "expected <%d> but was <%d>", expected, actual);
	CuFail_Line(tc, file, line, message, buf);
}


void CuAssertUIntEquals_LineMsg(CuTest *tc, const char *file,  int line,
                                const char *message,
                                unsigned int expected, unsigned int actual)
{
	char buf[STRING_MAX];
	if (expected == actual) return;
	sprintf(buf, "expected <%u> but was <%u>", expected, actual);
	CuFail_Line(tc, file, line, message, buf);
}



void CuAssertLongEquals_LineMsg(CuTest *tc, const char *file,  int line,
                                const char *message,
                                long expected, long actual)
{
	char buf[STRING_MAX];
	if (expected == actual) return;
	sprintf(buf, "expected <%ld> but was <%ld>", expected, actual);
	CuFail_Line(tc, file, line, message, buf);
}


void CuAssertULongEquals_LineMsg(CuTest *tc, const char *file,  int line,
                                 const char *message,
                                 unsigned long expected, unsigned long actual)
{
	char buf[STRING_MAX];
	if (expected == actual) return;
	sprintf(buf, "expected <%lu> but was <%lu>", expected, actual);
	CuFail_Line(tc, file, line, message, buf);
}


void CuAssertLlongEquals_LineMsg(CuTest *tc, const char *file,  int line,
                                 const char *message,
                                 long long expected, long long actual)
{
	char buf[STRING_MAX];
	if (expected == actual) return;
	sprintf(buf, "expected <%lld> but was <%lld>", expected, actual);
	CuFail_Line(tc, file, line, message, buf);
}


void CuAssertULlongEquals_LineMsg(CuTest *tc, const char *file, int line,
                                  const char *message,
                                  unsigned long long expected, unsigned long long actual)
{
	char buf[STRING_MAX];
	if (expected == actual) return;
	sprintf(buf, "expected <%llu> but was <%llu>", expected, actual);
	CuFail_Line(tc, file, line, message, buf);
}


void CuAssertSizeTEquals_LineMsg(CuTest *tc, const char *file, int line,
                                 const char *message,
                                 size_t expected, size_t actual)
{
	char buf[STRING_MAX];
	if (expected == actual) return;
	sprintf(buf, "expected <%zu> but was <%zu>", expected, actual);
	CuFail_Line(tc, file, line, message, buf);
}

void CuAssertDblEquals_LineMsg(CuTest *tc, const char *file, int line,
                               const char *message,
                               double expected, double actual, double delta)
{
	char buf[STRING_MAX];
	if (fabs(expected - actual) <= delta) return;
	sprintf(buf, "expected <%f> but was <%f>", expected, actual);

	CuFail_Line(tc, file, line, message, buf);
}

void CuAssertPtrEquals_LineMsg(CuTest *tc, const char *file, int line,
                               const char *message,
                               void *expected, void *actual)
{
	char buf[STRING_MAX];
	if (expected == actual) return;
	sprintf(buf, "expected pointer <0x%p> but was <0x%p>", expected, actual);
	CuFail_Line(tc, file, line, message, buf);
}


/*-------------------------------------------------------------------------*
 * CuSuite
 *-------------------------------------------------------------------------*/

void CuSuiteInit(CuSuite *testSuite, const char *suiteName)
{
	assert(testSuite);
	testSuite->name = CuStrCopy(suiteName);
	testSuite->testsCount = 0;
	testSuite->suitesCount = 0;
	memset(testSuite->tests, 0, sizeof(testSuite->tests));
	memset(testSuite->suites, 0, sizeof(testSuite->suites));
}

CuSuite *CuSuiteNew(const char *suiteName)
{
	CuSuite *testSuite = CU_ALLOC(CuSuite);
	CuSuiteInit(testSuite, suiteName);
	return testSuite;
}

void CuSuiteDelete(CuSuite *testSuite)
{
	for (int i = 0; i < MAX_SUITES; i++) {
		if (testSuite->suites[i]) {
			CuSuiteDelete(testSuite->suites[i]);
		}
	}
	for (int i = 0; i < MAX_TEST_CASES; i++) {
		if (testSuite->tests[i]) {
			CuTestDelete(testSuite->tests[i]);
		}
	}
	free(testSuite->name);
	free(testSuite);
}


bool CuSuiteAdd(CuSuite *testSuite, CuTest *testCase, const char* qualifiedName)
{
	if (testSuite->testsCount == MAX_TEST_CASES) {
		fprintf(stderr, "Cannot add test %s to %s: maximum number of tests reached.\n", testCase?testCase->name:"unnamed", testSuite?testSuite->name:"unnamed");
		return false;
	}
	testSuite->tests[testSuite->testsCount++] = testCase;
}


bool CuSuiteContainsDeep(CuSuite *parent, CuSuite *child)
{
	if (!parent || !child) return false;
	for (int i = 0; i < MAX_SUITES; ++i) {
		if (CuSuiteContainsDeep(parent->suites[i], child)) {
			return true;
		}
	}
	return false;
}

bool CuSuiteAddSuite(CuSuite *parentSuite, CuSuite *childSuite)
{
	if (parentSuite->suitesCount == MAX_SUITES) {
		fprintf(stderr, "Cannot add suite %s to %s: maximum number os suites reached.\n", childSuite?childSuite->name:"unnamed", parentSuite?parentSuite->name:"unnamed");
		return false;
	}
	// check for cycles
	if (CuSuiteContainsDeep(parentSuite, childSuite)) {
		fprintf(stderr, "Cannot add suite %s to %s: contains cycle.\n", childSuite?childSuite->name:"unnamed", parentSuite?parentSuite->name:"unnamed");
		return false;
	}
	parentSuite->suites[parentSuite->suitesCount++] = childSuite;
}


bool CuSuiteRun(CuSuite *testSuite)
{
	if (!testSuite) return true;
	bool ret = true;
	for (int i = 0; i < testSuite->suitesCount; i++) {
		ret = ret && CuSuiteRun(testSuite->suites[i]);
	}
	for (int i = 0 ; i < testSuite->testsCount ; ++i) {
		CuTest *testCase = testSuite->tests[i];
		CuTestRun(testCase);
		ret = ret && !testCase->failed;
	}
	return ret;
}


bool _CuSuiteRunSuite(CuSuite *testSuite, const char* suiteName, bool force)
{
	if (!testSuite) return true;
	bool ret = true;
	bool nameEquals = testSuite->name && !strcmp(testSuite->name, suiteName);
	for (int i = 0; i < testSuite->suitesCount; i++) {
		ret = ret && _CuSuiteRunSuite(testSuite->suites[i], suiteName, nameEquals);
	}
	for (int i = 0 ; i < testSuite->testsCount ; ++i) {
		if (force || nameEquals) {
			CuTest *testCase = testSuite->tests[i];
			CuTestRun(testCase);
			ret = ret && !testCase->failed;
		}
	}
	return ret;
}


bool CuSuiteRunSuite(CuSuite *testSuite, const char* suiteName)
{
	return _CuSuiteRunSuite(testSuite, suiteName, false);
}


typedef struct summresults {
	int ok;
	int fail;
	int skipped;
} summresults;

void _CuSuiteSummary(CuSuite *testSuite, CuString *summary, summresults *res)
{
	for (int i = 0; i < testSuite->suitesCount; ++i) {
		_CuSuiteSummary(testSuite->suites[i], summary, res);
	}
	for (int i = 0 ; i < testSuite->testsCount ; ++i) {
		CuTest *testCase = testSuite->tests[i];
		if (!testCase->ran) {
			CuStringAppend(summary,  ".");
			res->skipped++;
		} else {
			CuStringAppend(summary, testCase->failed ? "F" : "V");
			int failed = testCase->failed;
			res->ok += (1 - failed);
			res->fail += failed;
		}
	}
}


void _CuSuiteFailureDetails(CuSuite *testSuite, CuString *summary, int *failCount)
{
	for (int i = 0; i < testSuite->suitesCount; ++i) {
		_CuSuiteFailureDetails(testSuite->suites[i], summary, failCount);
	}
	for (int i = 0 ; i < testSuite->testsCount ; ++i) {
		CuTest *testCase = testSuite->tests[i];
		if (testCase->failed) {
			(*failCount)++;
			CuStringAppend(summary, testCase->failed ? "F" : "V");
			CuStringAppendFormat(summary, "%4d) %s: %s\n",
			                     *failCount, testCase->name, testCase->message);
		}
	}
}


void CuSuiteSummary(CuSuite *testSuite, CuString *summary)
{
	summresults res = {0, 0, 0};
	_CuSuiteSummary(testSuite, summary, &res);
	CuStringAppend(summary, "\n");
	if (res.fail > 0) {
		if (res.fail == 1)
			CuStringAppend(summary, "There was 1 failure:\n");
		else
			CuStringAppendFormat(summary, "There were %d failures:\n", res.fail);
		int failCount = 0;
		_CuSuiteFailureDetails(testSuite, summary, &failCount);
	}
	CuStringAppendFormat(summary, "Pass: %d    Fail: %d    Skipped: %d\n", res.ok, res.fail, res.skipped);
}


void CuSuitePrintResults(CuSuite *testSuite, FILE* stream)
{
	fprintf(stream, "Test Results for suite %s:\n", testSuite->name?testSuite->name:"Undefined");
	CuString *output = CuStringNew();
	CuSuiteSummary(testSuite, output);
	CuStringAppend(output, "\n");
	fprintf(stream, "%s", output->buffer);
}


void _CuSuitePrint(CuSuite *testSuite, int level)
{
	if (!testSuite) return;
	fprintf(stdout, "%*sSuite: %s\n", 2*level, "", testSuite->name?testSuite->name:"Undefined");
	for (int i = 0; i < testSuite->suitesCount; i++) {
		_CuSuitePrint(testSuite->suites[i], level+1);
	}
	for (int i = 0; i < testSuite->testsCount; i++) {
		CuTest *test = testSuite->tests[i];
		fprintf(stdout, "%*sTest: %s\n", 2*(level+1), "", test->name?test->name:"Undefined");
	}
}

void CuSuitePrint(CuSuite *testSuite)
{
	_CuSuitePrint(testSuite, 0);
}

#undef MEM_DEBUG_OFF
