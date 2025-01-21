
#ifndef KDECONNECTCORE_EXPORT_H
#define KDECONNECTCORE_EXPORT_H

#ifdef KDECONNECTCORE_STATIC_DEFINE
#  define KDECONNECTCORE_EXPORT
#  define KDECONNECTCORE_NO_EXPORT
#else
#  ifndef KDECONNECTCORE_EXPORT
#    ifdef kdeconnectcore_EXPORTS
        /* We are building this library */
#      define KDECONNECTCORE_EXPORT __declspec(dllexport)
#    else
        /* We are using this library */
#      define KDECONNECTCORE_EXPORT __declspec(dllimport)
#    endif
#  endif

#  ifndef KDECONNECTCORE_NO_EXPORT
#    define KDECONNECTCORE_NO_EXPORT 
#  endif
#endif

#ifndef KDECONNECTCORE_DEPRECATED
#  define KDECONNECTCORE_DEPRECATED __declspec(deprecated)
#endif

#ifndef KDECONNECTCORE_DEPRECATED_EXPORT
#  define KDECONNECTCORE_DEPRECATED_EXPORT KDECONNECTCORE_EXPORT KDECONNECTCORE_DEPRECATED
#endif

#ifndef KDECONNECTCORE_DEPRECATED_NO_EXPORT
#  define KDECONNECTCORE_DEPRECATED_NO_EXPORT KDECONNECTCORE_NO_EXPORT KDECONNECTCORE_DEPRECATED
#endif

/* NOLINTNEXTLINE(readability-avoid-unconditional-preprocessor-if) */
#if 0 /* DEFINE_NO_DEPRECATED */
#  ifndef KDECONNECTCORE_NO_DEPRECATED
#    define KDECONNECTCORE_NO_DEPRECATED
#  endif
#endif

#endif /* KDECONNECTCORE_EXPORT_H */
