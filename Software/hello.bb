SUMMARY = "Ein einfaches Hello-World-C-Programm"
LICENSE = "MIT"
LIC_FILES_CHKSUM = "file://${COMMON_LICENSE_DIR}/MIT;md5=0835c1a7d8daca70c69a8e6edbd9d3b2"

SRC_URI = "file://hello.c"
S = "${WORKDIR}"

do_compile() {
    ${CC} ${CFLAGS} ${LDFLAGS} -o hello hello.c
}

do_install() {
    install -d ${D}${bindir}
    install -m 0755 hello ${D}${bindir}
}
