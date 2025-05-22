qt_add_library(KF6Codecs SHARED
    kcodecs_debug.h
    kcodecs_debug.cpp
    kcharsets.cpp
    kcharsets.h
    kcodecsbase45.cpp
    kcodecsbase64.cpp
    kcodecsbase64.h
    kcodecs.cpp
    kcodecs.h
    kcodecs_p.h
    kcodecsqp.cpp
    kcodecsqp.h
    kcodecsuuencode.cpp
    kcodecsuuencode.h
    kemailaddress.cpp
    kemailaddress.h
    # kencodingprober.cpp
    # kencodingprober.h
    # probers/CharDistribution.cpp
    # probers/CharDistribution.h
    # probers/ChineseGroupProber.cpp
    # probers/ChineseGroupProber.h
    # probers/JapaneseGroupProber.cpp
    # probers/JapaneseGroupProber.h
    # probers/JpCntx.cpp
    # probers/JpCntx.h
    # probers/LangBulgarianModel.cpp
    # probers/LangCyrillicModel.cpp
    # probers/LangGreekModel.cpp
    # probers/LangHebrewModel.cpp
    # probers/LangHungarianModel.cpp
    # probers/LangThaiModel.cpp
    # probers/nsBig5Prober.cpp
    # probers/nsBig5Prober.h
    # probers/nsCharSetProber.cpp
    # probers/nsCharSetProber.h
    # probers/nsCodingStateMachine.h
    # probers/nsEscCharsetProber.cpp
    # probers/nsEscCharsetProber.h
    # probers/nsEscSM.cpp
    # probers/nsEUCJPProber.cpp
    # probers/nsEUCJPProber.h
    # probers/nsEUCKRProber.cpp
    # probers/nsEUCKRProber.h
    # probers/nsGB2312Prober.cpp
    # probers/nsGB2312Prober.h
    # probers/nsHebrewProber.cpp
    # probers/nsHebrewProber.h
    # probers/nsLatin1Prober.cpp
    # probers/nsLatin1Prober.h
    # probers/nsMBCSGroupProber.cpp
    # probers/nsMBCSGroupProber.h
    # probers/nsMBCSSM.cpp
    # probers/nsPkgInt.h
    # probers/nsSBCharSetProber.cpp
    # probers/nsSBCharSetProber.h
    # probers/nsSBCSGroupProber.cpp
    # probers/nsSBCSGroupProber.h
    # probers/nsSJISProber.cpp
    # probers/nsSJISProber.h
    # probers/nsUniversalDetector.cpp
    # probers/nsUniversalDetector.h
    # probers/UnicodeGroupProber.cpp
    # probers/UnicodeGroupProber.h
)

include(GenerateExportHeader)
generate_export_header(KF6Codecs BASE_NAME KCodecs)

target_include_directories(KF6Codecs PUBLIC
	"$<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}>"
	"$<BUILD_INTERFACE:${CMAKE_CURRENT_BINARY_DIR}>")
	
target_link_libraries(KF6Codecs PRIVATE Qt::Core)

include(GNUInstallDirs)
install(TARGETS KF6Codecs
    BUNDLE DESTINATION .
    LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
    RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
)
