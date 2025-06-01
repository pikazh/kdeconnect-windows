
set(backends_kdeconnect_SRCS
    ${backends_kdeconnect_SRCS}
    backends/lan/server.cpp
    backends/lan/server.h
    backends/lan/lanlinkprovider.cpp
    backends/lan/lanlinkprovider.h
    backends/lan/landevicelink.cpp
    backends/lan/landevicelink.h
    backends/lan/mdnsdiscovery.cpp
    backends/lan/mdnsdiscovery.h
    backends/lan/mdns_wrapper.cpp
    backends/lan/mdns_wrapper.h

    PARENT_SCOPE
)
