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
	HttpConnection_init(&conn, HTTP_REQ);

	HttpCodec_init(&decoder, &conn);

	int off = 0;
	int len = strlen(STR_REQ);
	int ret;
	int n;

	while(off < len) {
		if((n = len - off) > 64) {
			n = 64;
			ret = HttpCodec_decode(&decoder, STR_REQ + off, &n);
			assert_int_equal(n, 64);
			off += n;
			assert_int_equal(ret, EXIT_PEND);
		} else {
			ret = HttpCodec_decode(&decoder, STR_REQ + off, &n);
			assert_int_equal(n, len - off);
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
	HttpConnection_init(&conn, HTTP_RES);

	HttpCodec_init(&decoder, &conn);

	int off = 0;
	int len = strlen(STR_RES);
	int n;
	int ret;

	while(off < len) {
		if((n = len - off) > 64) {
			n = 64;
			ret = HttpCodec_decode(&decoder, STR_RES + off, &n);
			assert_int_equal(n, 64);
			off += n;
			assert_int_equal(ret, EXIT_PEND);
		} else {
			ret = HttpCodec_decode(&decoder, STR_RES + off, &n);
			assert_int_equal(n, len - off);
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

void test_put_get_header(void** state) {
	http_connection_t conn;
	HttpConnection_init(&conn, HTTP_REQ);
	HttpConnection_putHeader(&conn, "Host",       "xetn.io"   );
	HttpConnection_putHeader(&conn, "Server",     "XETN/1.1"  );
	HttpConnection_putHeader(&conn, "Connection", "keep-alive");
	HttpConnection_putHeader(&conn, "Keep-Alive", "163"       );
	HttpConnection_putHeader(&conn, "other1",     "test1"     );
	HttpConnection_putHeader(&conn, "other2",     "test2"     );
	HttpConnection_putHeader(&conn, "other3",     "test3"     );
	assert_memory_equal("xetn.io",    HttpConnection_getHeader(&conn, "Host"      ), 8 );
	assert_memory_equal("test1",      HttpConnection_getHeader(&conn, "other1"    ), 6 );
	assert_memory_equal("test2",      HttpConnection_getHeader(&conn, "other2"    ), 6 );
	assert_memory_equal("XETN/1.1",   HttpConnection_getHeader(&conn, "Server"    ), 9 );
	assert_memory_equal("keep-alive", HttpConnection_getHeader(&conn, "Connection"), 11);
	assert_memory_equal("163",        HttpConnection_getHeader(&conn, "Keep-Alive"), 4 );
	assert_memory_equal("test3",      HttpConnection_getHeader(&conn, "other3"    ), 6 );
}

void test_set_get_ver(void** state) {
	http_connection_t conn;
	HttpConnection_init(&conn, HTTP_REQ);
	HttpConnection_setVersion(&conn, HTTP1_1);
	assert_int_equal(HTTP1_1, HttpConnection_getVersion(&conn));
	assert_memory_equal("HTTP/1.1", HttpConnection_getVersionStr(&conn), 9);
}

void test_set_get_method(void** state) {
	http_connection_t conn;
	HttpConnection_init(&conn, HTTP_REQ);
	HttpConnection_setMethod(&conn, HTTP_GET);
	assert_int_equal(HTTP_GET, HttpConnection_getMethod(&conn));
	assert_memory_equal("GET", HttpConnection_getMethodStr(&conn), 4);
}

void test_put_get_path(void** state) {
	http_connection_t conn;
	HttpConnection_init(&conn, HTTP_REQ);

	char* url = "http://localhost:8080/test/home/codesun/path?name=codesun&age=24";
	HttpConnection_setPath(&conn, url);
	assert_memory_equal(url, HttpConnection_getPath(&conn), strlen(url) + 1);
}

int main() {
	const struct CMUnitTest tests[] = {
		//cmocka_unit_test_setup_teardown(),
		cmocka_unit_test(test_codec_req),
		cmocka_unit_test(test_codec_res),
		cmocka_unit_test(test_put_get_header),
		cmocka_unit_test(test_set_get_method),
		cmocka_unit_test(test_set_get_ver),
		cmocka_unit_test(test_put_get_path),
	};
	return cmocka_run_group_tests(tests, NULL, NULL);
}
