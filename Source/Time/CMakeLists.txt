# CMakeLists for Katydid/Source/Time
# Author: N. Oblath

set (TIME_HEADERFILES
    KTDAC.hh
    KTDigitizerTests.hh
    KTEggProcessor.hh
    KTEggReader.hh
    KTEggReader2011.hh
)

set (TIME_SOURCEFILES
    KTDAC.cc
    KTDigitizerTests.cc
    KTEggProcessor.cc
    KTEggReader.cc
    KTEggReader2011.cc
)

if (Katydid_USE_MONARCH)
    set (TIME_HEADERFILES
        ${TIME_HEADERFILES}
        KTEggReaderMonarch.hh
        #KTEggWriter.hh
    )
    set (TIME_SOURCEFILES
        ${TIME_SOURCEFILES}
        KTEggReaderMonarch.cc
        #KTEggWriter.cc
    )
endif (Katydid_USE_MONARCH)

if (Katydid_USE_MANTIS)
    set (TIME_HEADERFILES
        ${TIME_HEADERFILES}
        KTMantis.hh
        KTMantisClientWriting.hh
        KTMantisWriterToTS.hh
    )
    set (TIME_SOURCEFILES
        ${TIME_SOURCEFILES}
        KTMantis.cc
        KTMantisClientWriting.cc
        KTMantisWriterToTS.cc
    )
endif (Katydid_USE_MANTIS)

if (Katydid_USE_MATLAB)
    set (TIME_HEADERFILES
        ${TIME_HEADERFILES}
        KTRSAMatReader.hh
    )
    set (TIME_SOURCEFILES
        ${TIME_SOURCEFILES}
        KTRSAMatReader.cc
    )
endif (Katydid_USE_MATLAB)


set (KATYDID_LIBS
    KTCore
    KatydidUtility
    KatydidData
    KatydidIO
)

##################################################

add_library (KatydidTime ${TIME_SOURCEFILES})
target_link_libraries (KatydidTime ${KATYDID_LIBS} ${EXTERNAL_LIBRARIES} ${EXTERNAL_LIBRARIES_MATLAB})

pbuilder_install_headers (${TIME_HEADERFILES})
pbuilder_install_libraries (KatydidTime)
         