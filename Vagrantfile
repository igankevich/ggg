ENV['VAGRANT_DEFAULT_PROVIDER'] = 'virtualbox'

Vagrant.configure("2") do |config|
	config.vm.provider "virtualbox" do |v|
		v.memory = 1024
		v.cpus = 1
	end
	config.vm.box = "fedora/28-cloud-base"
	config.vm.box_version = "20180425"
	config.vm.synced_folder "/home/igankevich/rpmbuild/RPMS", "/rpms"
	config.vm.provision "shell", inline: <<-SHELL
	cat > /etc/nsswitch << EOF
passwd:     files ggg
group:      files ggg
netgroup:   files
automount:  files
services:   files
sudoers:    files

shadow:     files ggg
ethers:     files
netmasks:   files
networks:   files
protocols:  files
rpc:        files
hosts:      files dns myhostname

aliases:    files nisplus
bootparams: nisplus [NOTFOUND=return] files
publickey:  nisplus
EOF
	cat > /etc/pam.d/ggg << EOF
#%PAM-1.0
auth      required  pam_ggg.so debug entropy=0
account   required  pam_ggg.so debug entropy=0
password  required  pam_ggg.so debug entropy=0
session   optional  pam_ggg.so debug entropy=0
EOF
	cat > /etc/pam.d/password-auth-ggg << EOF
auth        required      pam_env.so
auth        required      pam_faildelay.so delay=2000000
auth        [default=1 ignore=ignore success=ok] pam_succeed_if.so uid >= 1000 quiet
auth        [default=1 ignore=ignore success=ok] pam_localuser.so
auth        sufficient    pam_unix.so nullok try_first_pass
auth        requisite     pam_succeed_if.so uid >= 1000 quiet_success
auth        sufficient    pam_ggg.so debug
auth        required      pam_deny.so

account     required      pam_unix.so
account     sufficient    pam_localuser.so
account     sufficient    pam_succeed_if.so uid < 1000 quiet
account     sufficient    pam_ggg.so debug entropy=0 register type=console
account     required      pam_deny.so

password    sufficient    pam_unix.so sha512 shadow nullok try_first_pass use_authtok
password    sufficient    pam_ggg.so debug entropy=0 type=console
password    required      pam_deny.so

session     optional      pam_keyinit.so revoke
session     required      pam_limits.so
-session     optional      pam_systemd.so
session     optional      pam_mkhomedir.so skel=/etc/skel/ umask=0077
session     optional      pam_script.so onerr=success
session     [success=1 default=ignore] pam_succeed_if.so service in crond quiet use_uid
session     required      pam_unix.so
session     optional      pam_ggg.so debug
EOF
	(cd /etc/pam.d && ln -sfn password-auth-ggg password-auth)
	SHELL
end

# vi:set ft=ruby:
