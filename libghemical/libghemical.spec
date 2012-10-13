Summary: The MM and QM calculations frontend.
Name: ghemical
Version: 1.50
Release: 1
Copyright: GPL
Group: Applications/Chemistry
Source: http://www.uku.fi/~thassine/ghemical/download/%{name}-%{version}.tgz
URL: http://www.uku.fi/~thassine/ghemical/
Buildroot: /var/tmp/%{name}-%{version}-%{release}-root
Prefix: /usr
Requires: gtkglarea
BuildRequires: gtkglarea
Icon: ghemical.xpm
Vendor: Tommi Hassinen et al.
Packager: Radek Liboska <liboska@uochb.cas.cz>

%description
Ghemical is a computational chemistry software package released under the 
GNU GPL.
Ghemical is written in C++. It has a graphical user interface (in fact, 
a couple of them), and it supports both quantum-mechanics (semi-empirical 
and ab initio) models and molecular mechanics models (there is an experimental
Tripos 5.2-like force field for organic molecules). Also a tool for reduced 
protein models is included. Geometry optimization, molecular dynamics
and a large set of visualization tools are currently available.

%prep
rm -rf $RPM_BUILD_ROOT

%setup
%build
./configure --prefix=%{prefix}
make

%install
install -d $RPM_BUILD_ROOT%prefix/bin/
install -d $RPM_BUILD_ROOT%prefix/share/%{name}/%{version}/parameters/{builder,mm2param,mm1param/stable,mm1param/unstable}
install -d $RPM_BUILD_ROOT%prefix/share/%{name}/%{version}/glade
install -d $RPM_BUILD_ROOT/usr/share/doc/%{name}-%{version}/examples
install -d $RPM_BUILD_ROOT/usr/share/gnome/help/%{name}/C
install -d $RPM_BUILD_ROOT/usr/share/gnome/help/%{name}/C/images
install -m 755 -s bin/ghemical $RPM_BUILD_ROOT%prefix/bin
cp -p -R bin/examples/* $RPM_BUILD_ROOT/usr/share/doc/%{name}-%{version}/examples
cp -p -R bin/user-docs/*.html $RPM_BUILD_ROOT/usr/share/gnome/help/%{name}/C
cp -p -R bin/user-docs/images/*.png $RPM_BUILD_ROOT/usr/share/gnome/help/%{name}/C/images
cp -p -R openbabel/*.txt  $RPM_BUILD_ROOT%prefix/share/%{name}/%{version}
cp -p -R bin/parameters  $RPM_BUILD_ROOT%prefix/share/%{name}/%{version}
cp -p src/target3/glade/*.glade  $RPM_BUILD_ROOT%prefix/share/%{name}/%{version}/glade

# remove CVS directories:
find $RPM_BUILD_ROOT/. -type d -name CVS | xargs rm -fR

%files
%defattr(-,root,root)
%doc /usr/share/doc/%{name}-%{version}
%doc /usr/share/gnome/help/%{name}
%prefix/bin/ghemical
%prefix/share/%{name}

%clean
rm -rf $RPM_BUILD_ROOT

%changelog
* Tue Dec 03 2002 Tommi Hassinen <thassine@messi.uku.fi>
- changes made for v1.00:
- updated the version number.
- modified the user-docs installation procedure (the images subdir).
* Thu Jun 20 2002 Tommi Hassinen <thassine@messi.uku.fi>
- changes made for v0.90:
- updated the version number.
- added the glade XML file installation.
* Wed Feb 27 2002 Radek Liboska <liboska@uochb.cas.cz>
- help files path fixed
- default path prefix changed to /usr
- minor changes in the header
- examples path relocated
* Mon Feb 25 2002 Radek Liboska <liboska@uochb.cas.cz>
- BuildRequires and Requires added (GtkGLArea)
- remove CVS directories from the rpm_build_root tree
* Fri Feb 22 2002 Radek Liboska <liboska@uochb.cas.cz>
- initial release for 0.82
