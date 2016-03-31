#ifndef _MIME_H_
#define _MIME_H_

#define MIME_MAP(XX) \
	XX("html"   , "text/html"                                                                ) \
	XX("htm"    , "text/html"                                                                ) \
	XX("shtml"  , "text/html"                                                                ) \
	XX("css"    , "text/css"                                                                 ) \
	XX("xml"    , "text/xml"                                                                 ) \
	XX("gif"    , "image/gif"                                                                ) \
	XX("jpeg"   , "image/jpeg"                                                               ) \
	XX("jpg"    , "image/jpeg"                                                               ) \
	XX("js"     , "application/javascript"                                                   ) \
	XX("atom"   , "application/atom+xml"                                                     ) \
	XX("rss"    , "application/rss+xml"                                                      ) \
	XX("mml"    , "text/mathml"                                                              ) \
	XX("txt"    , "text/plain"                                                               ) \
	XX("jad"    , "text/vnd.sun.j2me.app-descriptor"                                         ) \
	XX("wml"    , "text/vnd.wap.wml"                                                         ) \
	XX("htc"    , "text/x-component"                                                         ) \
	XX("png"    , "image/png"                                                                ) \
	XX("tif"    , "image/tiff"                                                               ) \
	XX("tiff"   , "image/tiff"                                                               ) \
	XX("wbmp"   , "image/vnd.wap.wbmp"                                                       ) \
	XX("ico"    , "image/x-icon"                                                             ) \
	XX("jng"    , "image/x-jng"                                                              ) \
	XX("bmp"    , "image/x-ms-bmp"                                                           ) \
	XX("svg"    , "image/svg+xml"                                                            ) \
	XX("svgz"   , "image/svg+xml"                                                            ) \
	XX("webp"   , "image/webp"                                                               ) \
	XX("woff"   , "application/font-woff"                                                    ) \
	XX("jar"    , "application/java-archive"                                                 ) \
	XX("war"    , "application/java-archive"                                                 ) \
	XX("ear"    , "application/java-archive"                                                 ) \
	XX("json"   , "application/json"                                                         ) \
	XX("hqx"    , "application/mac-binhex40"                                                 ) \
	XX("doc"    , "application/msword"                                                       ) \
	XX("pdf"    , "application/pdf"                                                          ) \
	XX("ps"     , "application/postscript"                                                   ) \
	XX("eps"    , "application/postscript"                                                   ) \
	XX("ai"     , "application/postscript"                                                   ) \
	XX("rtf"    , "application/rtf"                                                          ) \
	XX("m3u8"   , "application/vnd.apple.mpegurl"                                            ) \
	XX("xls"    , "application/vnd.ms-excel"                                                 ) \
	XX("eot"    , "application/vnd.ms-fontobject"                                            ) \
	XX("ppt"    , "application/vnd.ms-powerpoint"                                            ) \
	XX("wmlc"   , "application/vnd.wap.wmlc"                                                 ) \
	XX("kml"    , "application/vnd.google-earth.kml+xml"                                     ) \
	XX("kmz"    , "application/vnd.google-earth.kmz"                                         ) \
	XX("7z"     , "application/x-7z-compressed"                                              ) \
	XX("cco"    , "application/x-cocoa"                                                      ) \
	XX("jardiff", "application/x-java-archive-diff"                                          ) \
	XX("jnlp"   , "application/x-java-jnlp-file"                                             ) \
	XX("run"    , "application/x-makeself"                                                   ) \
	XX("pl"     , "application/x-perl"                                                       ) \
	XX("pm"     , "application/x-perl"                                                       ) \
	XX("prc"    , "application/x-pilot"                                                      ) \
	XX("pdb"    , "application/x-pilot"                                                      ) \
	XX("rar"    , "application/x-rar-compressed"                                             ) \
	XX("rpm"    , "application/x-redhat-package-manager"                                     ) \
	XX("sea"    , "application/x-sea"                                                        ) \
	XX("swf"    , "application/x-shockwave-flash"                                            ) \
	XX("sit"    , "application/x-stuffit"                                                    ) \
	XX("tcl"    , "application/x-tcl"                                                        ) \
	XX("tk"     , "application/x-tcl"                                                        ) \
	XX("der"    , "application/x-x509-ca-cert"                                               ) \
	XX("pem"    , "application/x-x509-ca-cert"                                               ) \
	XX("crt"    , "application/x-x509-ca-cert"                                               ) \
	XX("xpi"    , "application/x-xpinstall"                                                  ) \
	XX("xhtml"  , "application/xhtml+xml"                                                    ) \
	XX("xspf"   , "application/xspf+xml"                                                     ) \
	XX("zip"    , "application/zip"                                                          ) \
	XX("bin"    , "application/octet-stream"                                                 ) \
	XX("exe"    , "application/octet-stream"                                                 ) \
	XX("dll"    , "application/octet-stream"                                                 ) \
	XX("deb"    , "application/octet-stream"                                                 ) \
	XX("dmg"    , "application/octet-stream"                                                 ) \
	XX("img"    , "application/octet-stream"                                                 ) \
	XX("iso"    , "application/octet-stream"                                                 ) \
	XX("msi"    , "application/octet-stream"                                                 ) \
	XX("msp"    , "application/octet-stream"                                                 ) \
	XX("msm"    , "application/octet-stream"                                                 ) \
	XX("docx"   , "application/vnd.openxmlformats-officedocument.wordprocessingml.document"  ) \
	XX("xlsx"   , "application/vnd.openxmlformats-officedocument.spreadsheetml.sheet"        ) \
	XX("pptx"   , "application/vnd.openxmlformats-officedocument.presentationml.presentation") \
	XX("mid"    , "audio/midi"                                                               ) \
	XX("midi"   , "audio/midi"                                                               ) \
	XX("kar"    , "audio/midi"                                                               ) \
	XX("mp3"    , "audio/mpeg"                                                               ) \
	XX("ogg"    , "audio/ogg"                                                                ) \
	XX("m4a"    , "audio/x-m4a"                                                              ) \
	XX("ra"     , "audio/x-realaudio"                                                        ) \
	XX("3gpp"   , "video/3gpp"                                                               ) \
	XX("3gp"    , "video/3gpp"                                                               ) \
	XX("ts"     , "video/mp2t"                                                               ) \
	XX("mp4"    , "video/mp4"                                                                ) \
	XX("mpeg"   , "video/mpeg"                                                               ) \
	XX("mpg"    , "video/mpeg"                                                               ) \
	XX("mov"    , "video/quicktime"                                                          ) \
	XX("webm"   , "video/webm"                                                               ) \
	XX("flv"    , "video/x-flv"                                                              ) \
	XX("m4v"    , "video/x-m4v"                                                              ) \
	XX("mng"    , "video/x-mng"                                                              ) \
	XX("asx"    , "video/x-ms-asf"                                                           ) \
	XX("asf"    , "video/x-ms-asf"                                                           ) \
	XX("wmv"    , "video/x-ms-wmv"                                                           ) \
	XX("avi"    , "video/x-msvideo"                                                          )

#define MIME_NUM 103

void        Mime_initModule();
void        Mime_closeModule();

const char* Mime_lookupType(const char*);
//const char* Mime_lookupSuffix(const char*);

#endif //_MIME_H_
