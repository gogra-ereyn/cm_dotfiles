include(GNUInstallDirs)

install(TARGETS mytool
    RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}      # executables -> bin/
    LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}      # shared libs -> lib/
    ARCHIVE DESTINATION ${CMAKE_INSTALL_LIBDIR}      # static libs -> lib/
)

install(FILES somefile.conf
    DESTINATION ${CMAKE_INSTALL_SYSCONFDIR}/mytool)  # config -> etc/

install(FILES mytool.1
    DESTINATION ${CMAKE_INSTALL_MANDIR}/man1)        # man pages
