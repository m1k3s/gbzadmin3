# Maintainer: Michael Sheppard <pbrane0@gmail.com>
# PILab3 build for local directory

pkgname=gbzadmin3
pkgver=2.4.2
pkgrel=2
pkgdesc="gBZAdmin, Gtkmm admin for bzflag"
arch=('i686' 'x86_64')
url=""
license=('GPL3')
depends=('gtkmm' 'glibmm')

build() {
   cd ..

   ./configure --prefix=/usr
   make
}

package() {
   cd ..

   make DESTDIR="${pkgdir}" install
   install -Dm644 "pixmaps/observer_icon.png"    "$pkgdir/usr/share/icons/hicolor/symbolic/apps/observer_icon.png"
   install -Dm644 "gbzadmin.desktop"    "$pkgdir/usr/share/applications/gbzadmin.desktop"
}
