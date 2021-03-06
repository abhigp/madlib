# ------------------------------------------------------------------------------
# Packaging with RPM
# ------------------------------------------------------------------------------
#
# Important: Set variables using set(... PARENT_SCOPE), so that the scope of the
# definition extends to the parent scope


# Get information about the environment
rh_version(RH_VERSION)

# -- Set RPM-specific variables ------------------------------------------------

set(CPACK_RPM_PACKAGE_ARCHITECTURE x86_64 PARENT_SCOPE)
set(CPACK_RPM_PACKAGE_LICENSE "New BSD License" PARENT_SCOPE)
set(CPACK_RPM_PACKAGE_GROUP "Development/Libraries" PARENT_SCOPE)
set(CPACK_PACKAGING_INSTALL_PREFIX "/usr/local/madlib" PARENT_SCOPE)

# gppkg uses a separate database than the rpm database present on the system and
# hence has no knowledge of the programs installed on the system. So when you
# try to install this package, you will get an error saying that /bin/sh has not been
# installed. This line tells rpm that the required dependency is satisfied by
# the package itself and will prevent rpm from erroring out. The /bin/sh dependency
# is automatically added as a dependency by the rpm when there are post/pre
# hooks in the spec file.

if(GPPKG_BUILD)
    set(CPACK_RPM_PACKAGE_PROVIDES "/bin/sh" PARENT_SCOPE)
else()
    set(_PACKAGE_REQUIRES "lapack >= 3.0, m4 >= 1.4")
    if(RH_VERSION AND RH_VERSION VERSION_LESS "6.0")
        # on RH/CentOS 5, there is no Python 2.6 or higher in the defaul
        # repositories.
        set(CPACK_RPM_PACKAGE_REQUIRES "python, ${_PACKAGE_REQUIRES}" PARENT_SCOPE)
    else()
        set(CPACK_RPM_PACKAGE_REQUIRES "python >= 2.6, ${_PACKAGE_REQUIRES}" PARENT_SCOPE)
    endif()
endif()

# We do all deployment preparation with our CMake build script, so we do
# not want to do any post-processing as part of the RPM generation.
if(GPPKG_BUILD)
   set(CPACK_RPM_SPEC_MORE_DEFINE "%undefine __os_install_post\nAutoReq:no" PARENT_SCOPE)
else()
   set(CPACK_RPM_SPEC_MORE_DEFINE "%undefine __os_install_post" PARENT_SCOPE)
endif()
