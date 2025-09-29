SUMMARY = "Sammelpaket mit C-Tools (Hello, AUS/LADEN/ENTLADEN, Analogwerte, Zustandsautomat)"
LICENSE = "MIT"
LIC_FILES_CHKSUM = "file://${COMMON_LICENSE_DIR}/MIT;md5=0835c1a7d8daca70c69a8e6edbd9d3b2"

SRC_URI = " \
    file://hello.c \
    file://aus.c \
    file://laden.c \
    file://entladen.c \
    file://analogwerte.c \
    file://zustandsautomat.c \
"

S = "${WORKDIR}"

do_compile() {
    ${CC} ${CFLAGS} ${LDFLAGS} -o hello hello.c
    ${CC} ${CFLAGS} ${LDFLAGS} -o aus aus.c
    ${CC} ${CFLAGS} ${LDFLAGS} -o laden laden.c
    ${CC} ${CFLAGS} ${LDFLAGS} -o entladen entladen.c
    ${CC} ${CFLAGS} ${LDFLAGS} -o analogwerte analogwerte.c
    ${CC} ${CFLAGS} ${LDFLAGS} -o zustandsautomat zustandsautomat.c
}

do_install() {
    install -d ${D}${bindir}
    install -m 0755 hello           ${D}${bindir}/hello
    install -m 0755 aus             ${D}${bindir}/aus
    install -m 0755 laden           ${D}${bindir}/laden
    install -m 0755 entladen        ${D}${bindir}/entladen
    install -m 0755 analogwerte     ${D}${bindir}/analogwerte
    install -m 0755 zustandsautomat ${D}${bindir}/zustandsautomat
}

FILES:${PN} += " \
    ${bindir}/hello \
    ${bindir}/aus \
    ${bindir}/laden \
    ${bindir}/entladen \
    ${bindir}/analogwerte \
    ${bindir}/zustandsautomat \
"
