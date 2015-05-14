lockdown', 'disable-lock-screen', 'true'),
                    ('org.gnome.desktop.lockdown', 'disable-user-switching', 'true'),
                ]
                for gs_schema, gs_key, gs_value in gsettings_keys:
                    gsettings.set(gs_schema, gs_key, gs_value, self.username)

                accessibility = gsettings.get('org.gnome.desktop.interface',
                    'toolkit-accessibility', self.username)
            # Accessibility infrastructure
            with open('/proc/cmdline', 'r') as fp:
                if (accessibility == True or 'maybe-ubiquity' in fp.readline()):
                    if os.path.exists('/usr/lib/at-spi2-core/at-spi-bus-launcher'):
                        extras.append(subprocess.Popen(
                            ['/usr/lib/at-spi2-core/at-spi-bus-launcher',
                            '--launch-immediately'],
                            stdin=null, stdout=logfile, stderr=logfile,
                            preexec_fn=self.drop_privileges))
                        os.environ['GTK_MODULES'] = 'gail:atk-bridge'

            # Set a desktop wallpaper.
            with open('/proc/cmdline', 'r') as fp:
                for background in ('/usr/share/backgrounds/edubuntu_default.png',
                    '/usr/share/xfce4/backdrops/macinnis_wallpaper.png',
                    '/usr/share/xfce4/backdrops/xubuntu-precise-right.png',
                    '/usr/share/backgrounds/warty-final-ubuntu.png'):
                    exists = os.access(background, os.R_OK)
                    if (exists and not 'access=v1' in fp.readline()):
                        extras.append(subprocess.Popen(['/usr/lib/ubiquity/wallpaper', background],
                                stdin=null, stdout=logfile, stderr=logfile,
                                preexec_fn=self.drop_privileges))
                        break

        if self.frontend == 'gtk_ui':
            if gconfd_running and osextras.find_on_path('metacity'):
                wm_cmd = ['metacity', '--sm-disable']
            elif osextras.find_on_path('xfwm4'):
                wm_cmd = ['xfwm4']
            elif osextras.find_on_path('matchbox-window-manager'):
                wm_cmd = ['matchbox-window-manager']
            elif osextras.find_on_path('openbox-lubuntu'):
                wm_cmd = ['openbox-lubuntu']
            elif osextras.find_on_path('openbox'):
                wm_cmd = ['openbox']
            else:
                raise MissingProgramError('No window manager found (tried '
                    'metacity, xfwm4, matchbox-window-manager, '
                    'openbox-lubuntu, openbox)')
            wm = subprocess.Popen(wm_cmd,
                stdin=null, stdout=logfile, stderr=logfile,
                preexec_fn=self.drop_privileges)

            if os.path.exists('/usr/lib/gnome-settings-daemon/gnome-settings-daemon'):
                proc = subprocess.Popen(
                    ['/usr/lib/gnome-settings-daemon/gnome-settings-daemon'],
                    stdin=null, stdout=logfile, stderr=logfile,
                    preexec_fn=self.drop_privileges)
                extras.append(proc)

            if os.path.exists('/usr/bin/xfsettingsd'):
                extras.append(subprocess.Popen(
                    ['xsetroot', '-solid', 'black'],
                    stdin=null, stdout=logfile, stderr=logfile,
                    preexec_fn=self.drop_privileges))
                extras.append(subprocess.Popen(
                    ['/usr/bin/xfsettingsd'],
                    stdin=null, stdout=logfile, stderr=logfile,
                    preexec_fn=self.drop_privileges))

            if osextras.find_on_path('lxsession'):
                proc = subprocess.Popen(
                    ['lxsession','-s','Lubuntu', '-e', 'LXDE', '-a'],
                    stdin=null, stdout=logfile, stderr=logfile,
                    preexec_fn=self.drop_privileges)
                extras.append(proc)

            if os.path.exists('/usr/lib/ubiquity/panel') and "xfwm4" not in wm_cmd:
                if "openbox-lubuntu" not in wm_cmd and "openbox" not in wm_cmd:
                    extras.append(subprocess.Popen(
                        ['/usr/lib/ubiquity/panel'],
                        stdin=null, stdout=logfile, stderr=logfile,
                        preexec_fn=self.drop_privileges))

            if osextras.find_on_path('nm-applet'):
                extras.append(subprocess.Popen(
                    ['nm-applet'],
                    stdin=null, stdout=logfile, stderr=logfile,
                    preexec_fn=self.drop_privileges))

            if osextras.find_on_path('ibus-daemon'):
                extras.append(subprocess.Popen(
                    ['ibus-daemon'],
                    stdin=null, stdout=logfile, stderr=logfile,
                    preexec_fn=self.drop_privileges))

            # Simply start bluetooth-applet, ubiquity-bluetooth-agent will
            # override it from casper to make sure it also covers the regular
            # live session
            if osextras.find_on_path('bluetooth-applet'):
                extras.append(subprocess.Popen(
                    ['bluetooth-applet'],
                    stdin=null, stdout=logfile, stderr=logfile,
                    preexec_fn=self.drop_privileges))

            # Accessibility tools
            if accessibility == True:
                with open('/proc/cmdline', 'r') as fp:
                    if 'access=m2' in fp.readline():
                        if osextras.find_on_path('onboard'):
                            extras.append(subprocess.Popen(['onboard'],
                                stdin=null, stdout=logfile, stderr=logfile,
                                preexec_fn=self.drop_privileges))
                    else:
                        if osextras.find_on_path('orca'):
                            time.sleep(15)
                            extras.append(subprocess.Popen(['orca', '-n'],
                                stdin=null, stdout=logfile, stderr=logfile,
                                preexec_fn=self.drop_privileges))
        elif self.frontend == 'kde_ui':
            # Don't show the background image in v1 accessibility profile.
            os.setegid(self.gid)
            os.seteuid(self.uid)
            path = '/usr/share/wallpapers/kde-default.png'
            with open('/proc/cmdline', 'r') as fp:
                if (os.access(path, os.R_OK)
                    and not 'access=v1' in fp.readline()):
                    # Draw a background image for the install progress window.
                    from PyQt4.QtCore import Qt
                    from PyQt4 import QtGui
                    a = QtGui.QApplication(sys.argv)
                    root_win = a.desktop()
                    wallpaper = QtGui.QPixmap(path)
                    wallpaper = wallpaper.scaled(root_win.width(),
                        root_win.height(), Qt.KeepAspectRatioByExpanding,
                        Qt.SmoothTransformation)
                    palette = QtGui.QPalette()
                    palette.setBrush(root_win.backgroundRole(),
                                     QtGui.QBrush(wallpaper))
                    root_win.setPalette(palette)
                    root_win.setCursor(Qt.ArrowCursor)
                    a.processEvents()
                    # Force garbage collection so we don't end up with stray X
                    # resources when we kill the X server (LP: #556555).
                    del a
            os.seteuid(0)
            os.setegid(0)

            subprocess.call(
                ['kwriteconfig', '--file', 'kwinrc', '--group', 'Compositing',
                                 '--key', 'Enabled', 'false'],
                stdin=null, stdout=logfile, stderr=logfile,
                preexec_fn=self.drop_privileges)
            wm = subprocess.Popen('kwin',
                stdin=null, stdout=logfile, stderr=logfile,
                preexec_fn=self.drop_privileges)

        greeter = subprocess.Popen(program, stdin=null, stdout=logfile, stderr=logfile)
        ret = greeter.wait()

        reboot = False
        if ret != 0:
            db = DebconfCommunicator('ubiquity', cloexec=True)
            try:
                error_cmd = db.get('ubiquity/failure_command')
                if error_cmd:
                    subprocess.call(['sh', '-c', error_cmd])
            except debconf.DebconfError:
                pass

            reboot = False
            try:
                if '--automatic' in program:
                    reboot = db.get('ubiquity/reboot_on_failure') == 'true'
            except debconf.DebconfError:
                pass

            if reboot:
                question = 'ubiquity/install_failed_reboot'
            else:
                question = 'ubiquity/install_failed'
            title = ''
            message = ''
            try:
                title = unicode(db.metaget(question, 'description'),
                                'utf-8', 'replace')
                message = unicode(db.metaget(question, 'extended_description'),
                                  'utf-8', 'replace')
            except debconf.DebconfError:
                pass
            db.shutdown()

            if title and message:
                if self.frontend == 'gtk_ui':
                    cmd = ['zenity', '--error', '--title=%s' % title,
                           '--text=%s' % message]
                    subprocess.call(cmd)
                elif self.frontend == 'kde_ui':
                    cmd = ['kdialog',