$cp_content = <<SCRIPT
cp -rf /vagrant/* ./
rm Vagrantfile
SCRIPT

$tools_install = <<SCRIPT
sudo apt-get -y update
sudo apt-get -y install make gcc g++ mininet meson libmnl-dev
SCRIPT

$p4c_install = <<SCRIPT
. /etc/os-release
echo "deb https://download.opensuse.org/repositories/home:/p4lang/xUbuntu_${VERSION_ID}/ /" | sudo tee /etc/apt/sources.list.d/home:p4lang.list
curl -L "https://download.opensuse.org/repositories/home:/p4lang/xUbuntu_${VERSION_ID}/Release.key" | sudo apt-key add -
sudo apt-get -y update
sudo apt-get -y install p4lang-p4c
SCRIPT

$p4_build = <<SCRIPT
make -f p4c/backends/ebpf/runtime/kernel.mk \
  BPFOBJ=p4_router.o \
  P4FILE=router/top.p4 \
  ARGS="-DPSA_PORT_RECIRCULATE=2" \
  P4ARGS="--Wdisable=unused" \
  psa
SCRIPT

$nikss_build = <<SCRIPT
sudo apt-get install -y make cmake gcc git libgmp-dev libelf-dev zlib1g-dev libjansson-dev
cd nikss
./build_libbpf.sh
mkdir build
cd build
cmake -DCMAKE_INSTALL_PREFIX=/usr -DBUILD_SHARED=on ..
make -j4
sudo make install
sudo make install_headers
sudo ldconfig
SCRIPT

Vagrant.configure("2") do |config|
  config.vm.box = "bento/ubuntu-22.04"
  config.vm.provider "virtualbox"

  config.vm.provision "cp_content", type: 'shell', privileged: false,  inline: $cp_content
  config.vm.provision "tools_install", type: 'shell', privileged: false,  inline: $tools_install
  config.vm.provision "p4c_install", type: 'shell', privileged: false,  inline: $p4c_install
  config.vm.provision "p4_build", type: 'shell', privileged: false,  inline: $p4_build
  config.vm.provision "nikss_build", type: 'shell', privileged: false,  inline: $nikss_build
end
