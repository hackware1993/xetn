#ifndef _HTTP_HEADER_H_
#define _HTTP_HEADER_H_

#include <stdint.h>

#define HEADER_MAP(XX)                     \
	XX(HH_CONN , 10, "Connection"         ) \
	XX(HH_DATE ,  4, "Date"               ) \
	XX(HH_MMVER, 12, "MIME-Version"       ) \
	XX(HH_TRAL ,  7, "Trailer"            ) \
	XX(HH_TSEN , 17, "Transfer-Encoding"  ) \
	XX(HH_UPDT ,  6, "Update"             ) \
	XX(HH_VIA  ,  3, "Via"                ) \
	XX(HH_CACTL, 13, "Cache-Control"      ) \
	XX(HH_PGMA ,  6, "Pragma"             ) \
	XX(HH_ALOW ,  5, "Allow"              ) \
	XX(HH_LOCA ,  8, "Location"           ) \
	XX(HH_CTBA , 12, "Content-Base"       ) \
	XX(HH_CTEN , 16, "Content-Encoding"   ) \
	XX(HH_CTLG , 16, "Content-Language"   ) \
	XX(HH_CTLEN, 14, "Content-Length"     ) \
	XX(HH_CTLOC, 16, "Content-Location"   ) \
	XX(HH_CTMD5, 11, "Content-MD5"        ) \
	XX(HH_CTRNG, 13, "Content-Range"      ) \
	XX(HH_CTTPE, 12, "Content-Type"       ) \
	XX(HH_ETAG ,  4, "ETag"               ) \
	XX(HH_EXPR ,  7, "Expires"            ) \
	XX(HH_LMOD , 13, "Last-Modified"      ) \
	XX(HH_CLIP ,  9, "Client-IP"          ) \
	XX(HH_FROM ,  4, "From"               ) \
	XX(HH_HOST ,  4, "Host"               ) \
	XX(HH_REFER,  7, "Referer"            ) \
	XX(HH_UACLR,  8, "UA-Color"           ) \
	XX(HH_UACPU,  6, "UA-CPU"             ) \
	XX(HH_UADIS,  7, "UA-Disp"            ) \
	XX(HH_UAOS ,  5, "UA-OS"              ) \
	XX(HH_UAPIX,  9, "UA-Pixels"          ) \
	XX(HH_UA   , 10, "User-Agent"         ) \
	XX(HH_AUTH , 13, "Authorization"      ) \
	XX(HH_COOK ,  6, "Cookie"             ) \
	XX(HH_COOK2,  7, "Cookie2"            ) \
	XX(HH_ACPT ,  6, "Accept"             ) \
	XX(HH_ACCH , 14, "Accept-Charset"     ) \
	XX(HH_ACEN , 15, "Accept-Encoding"    ) \
	XX(HH_ACLG , 15, "Accept-Language"    ) \
	XX(HH_TE   ,  2, "TE"                 ) \
	XX(HH_EXPT ,  6, "Expect"             ) \
	XX(HH_IFMT ,  8, "If-Match"           ) \
	XX(HH_IFMS , 17, "If-Modified-Since"  ) \
	XX(HH_IFNM , 13, "If-None-Match"      ) \
	XX(HH_IFRG ,  8, "If-Range"           ) \
	XX(HH_IFUMS, 19, "If-Unmodified-Since") \
	XX(HH_RANGE,  5, "Range"              ) \
	XX(HH_MAXF , 11, "Max-Forward"        ) \
	XX(HH_PXAUT, 19, "Proxy-Authorization") \
	XX(HH_PXCON, 16, "Proxy-Connection"   ) \
	XX(HH_AGE  ,  3, "Age"                ) \
	XX(HH_PUB  ,  6, "Public"             ) \
	XX(HH_RTYAF, 11, "Retry-After"        ) \
	XX(HH_SERV ,  6, "Server"             ) \
	XX(HH_TITLE,  5, "Title"              ) \
	XX(HH_WARN ,  7, "Warning"            ) \
	XX(HH_ACRG,  13, "Accept-Ranges"      ) \
	XX(HH_VARY ,  4, "Vary"               ) \
	XX(HH_PXAET, 18, "Proxy-Authenticate" ) \
	XX(HH_SETC , 10, "Set-Cookie"         ) \
	XX(HH_SETC2, 11, "Set-Cookie2"        ) \
	XX(HH_W3AUT, 16, "WWW-Authenticate"   )  

#define XX(a, b, c) a,
typedef enum http_header {
	HEADER_MAP(XX)
	HH_INVALID,
} http_header_t;
#undef XX

#define HTTP_HEADER_NUM 62

http_header_t HttpHeader_findByHash(int32_t);
http_header_t HttpHeader_findByStr(const char*);
const char*   HttpHeader_getName(http_header_t);
uint8_t       HttpHeader_getLength(http_header_t);

#endif // _HTTP_HEADER_H_
