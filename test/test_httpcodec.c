#include "../src/http/codec.h"
#include "../src/http/connection.h"

#include <stdarg.h>
#include <stddef.h>
#include <setjmp.h>
#include <cmocka.h>

#include <string.h>
#define STR_RES \
    "HTTP/1.1 200 OK\r\n"                                                                                              \
    "Connection: keep-alive\r\n"                                                                                       \
	"Keep-Alive: 163\r\n"                                                                                              \
    "Host: www.kittyhell.com\r\n"                                                                                      \
    "User-Agent: Mozilla/5.0 (Macintosh; U; Intel Mac OS X 10.6; ja-JP-mac; rv:1.9.2.3) Gecko/20100401 Firefox/3.6.3 " \
    "Pathtraq/0.9\r\n"                                                                                                 \
    "Cookie: wp_ozh_wsa_visits=2; wp_ozh_wsa_visit_lasttime=xxxxxxxxxx; "                                              \
    "__utma=xxxxxxxxx.xxxxxxxxxx.xxxxxxxxxx.xxxxxxxxxx.xxxxxxxxxx.x; "                                                 \
    "__utmz=xxxxxxxxx.xxxxxxxxxx.x.x.utmccn=(referral)|utmcsr=reader.livedoor.com|utmcct=/reader/|utmcmd=referral\r\n" \
    "Accept: text/html,application/xhtml+xml,application/xml;q=0.9,*/*;q=0.8\r\n"                                      \
    "Accept-Charset: Shift_JIS,utf-8;q=0.7,*;q=0.7\r\n"                                                                \
    "Accept-Encoding: gzip,deflate\r\n"                                                                                \
    "Accept-Language: ja,en-us;q=0.7,en;q=0.3\r\n"                                                                     \
	"test: i'm codesun, the author of xetn, you know ha? this line is just for test the performance of my HTTP parser\r\n" \
    "\r\n"

#define STR_REQ \
	"GET /wp-content/uploads/2010/03/hello-kitty-darth-vader-pink.jpg HTTP/1.1\r\n"                                    \
    "Connection: keep-alive\r\n"                                                                                       \
	"Keep-Alive: 163\r\n"                                                                                              \
    "Host: www.kittyhell.com\r\n"                                                                                      \
    "User-Agent: Mozilla/5.0 (Macintosh; U; Intel Mac OS X 10.6; ja-JP-mac; rv:1.9.2.3) Gecko/20100401 Firefox/3.6.3 " \
    "Pathtraq/0.9\r\n"                                                                                                 \
    "Cookie: wp_ozh_wsa_visits=2; wp_ozh_wsa_visit_lasttime=xxxxxxxxxx; "                                              \
    "__utma=xxxxxxxxx.xxxxxxxxxx.xxxxxxxxxx.xxxxxxxxxx.xxxxxxxxxx.x; "                                                 \
    "__utmz=xxxxxxxxx.xxxxxxxxxx.x.x.utmccn=(referral)|utmcsr=reader.livedoor.com|utmcct=/reader/|utmcmd=referral\r\n" \
    "Accept: text/html,application/xhtml+xml,application/xml;q=0.9,*/*;q=0.8\r\n"                                      \
    "Accept-Charset: Shift_JIS,utf-8;q=0.7,*;q=0.7\r\n"                                                                \
    "Accept-Encoding: gzip,deflate\r\n"                                                                                \
    "Accept-Language: ja,en-us;q=0.7,en;q=0.3\r\n"                                                                     \
	"test: i'm codesun, the author of xetn, you know ha? this line is just for test the performance of my HTTP parser\r\n" \
    "\r\n"

int test_setup(void** state) {
	return 0;
}

int test_teardown(void** state) {
	return 0;
}

void test_codec_req(void** state) {
	http_codec_t decoder, encoder;
	http_connection_t conn;

	HttpConnection_init(&conn);
	conn.type = HTTP_REQ;

	HttpCodec_init(&decoder, &conn);

	int off = 0;
	int len = strlen(STR_REQ);
	int ret;

	while(off < len) {
		if(len - off > 64) {
			ret = HttpCodec_decode(&decoder, STR_REQ + off, 64);
			off += 64;
			assert_int_equal(ret, EXIT_PEND);
		} else {
			ret = HttpCodec_decode(&decoder, STR_REQ + off, len - off);
			off = len;
			assert_int_equal(ret, EXIT_DONE);
		}
	}
	assert_int_equal(off, len);

	HttpCodec_init(&encoder, &conn);

	off = 0;
	char str[4096] = {0};

	while(off < len) {
		ret = HttpCodec_encode(&encoder, str + off, &len);
		off += len;
	}
	assert_int_equal(ret, EXIT_DONE);
	assert_memory_equal(STR_REQ, str, len + 1);
}

void test_codec_res(void** state) {
	http_codec_t decoder, encoder;
	http_connection_t conn;

	HttpConnection_init(&conn);
	conn.type = HTTP_RES;

	HttpCodec_init(&decoder, &conn);

	int off = 0;
	int len = strlen(STR_RES);
	int ret;

	while(off < len) {
		if(len - off > 64) {
			ret = HttpCodec_decode(&decoder, STR_RES + off, 64);
			off += 64;
			assert_int_equal(ret, EXIT_PEND);
		} else {
			ret = HttpCodec_decode(&decoder, STR_RES + off, len - off);
			off = len;
			assert_int_equal(ret, EXIT_DONE);
		}
	}
	assert_int_equal(off, len);

	HttpCodec_init(&encoder, &conn);

	off = 0;
	char str[4096] = {0};

	while(off < len) {
		ret = HttpCodec_encode(&encoder, str + off, &len);
		off += len;
	}
	assert_int_equal(ret, EXIT_DONE);
	assert_memory_equal(STR_RES, str, len + 1);
}

int main() {
	const struct CMUnitTest tests[] = {
		//cmocka_unit_test_setup_teardown(),
		cmocka_unit_test(test_codec_req),
		cmocka_unit_test(test_codec_res),
	};
	return cmocka_run_group_tests(tests, NULL, NULL);
}
