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
cd ..
cp ../nikss_headers_fix.sh ./
./nikss_headers_fix.sh
cd build
sudo make install_headers
sudo ldconfig
cd
SCRIPT

$agent_build = <<SCRIPT
cd agent
meson setup build
ninja -C build
cd
SCRIPT

$ipv4_forwarding_enable = <<SCRIPT
sudo sysctl -w net.ipv4.ip_forward=1
SCRIPT

$host1_route = <<SCRIPT
sudo ip r add 192.168.200.0/24 via 192.168.100.1
SCRIPT

$host2_route = <<SCRIPT
sudo ip r add 192.168.100.0/24 via 192.168.200.1
SCRIPT

$host_setup = <<SCRIPT
sudo apt-get update
sudo apt-get install -y gcc g++ make

cp /vagrant/iperf-2.1.9.tar.gz ./
tar -xvf iperf-2.1.9.tar.gz
cd iperf-2.1.9
./configure
make
sudo make install
cd
SCRIPT

servers=[
  {
    :hostname => "host1",
    :ip => [
      {:name => "int_net1", :addr => "192.168.100.11"}
    ],
    :box => "minimal/xenial64",
    :ram => 512,
    :cpu => 1,
    :scripts => [
      {:name => "route", :script => $host1_route},
      {:name => "host_setup", :script => $host_setup}
    ]
  },
  {
    :hostname => "host2",
    :ip => [
      {:name => "int_net2", :addr => "192.168.200.12"}
    ],
    :box => "minimal/xenial64",
    :ram => 512,
    :cpu => 1,
    :scripts => [
      {:name => "route", :script => $host2_route},
      {:name => "host_setup", :script => $host_setup}
    ]
  },
  {
    :hostname => "router",
    :ip => [
      {:name => "int_net1", :addr => "192.168.100.1"},
      {:name => "int_net2", :addr => "192.168.200.1"}
    ],
    :box => "bento/ubuntu-22.04",
    :ram => 2048,
    :cpu => 2,
    :scripts => [
      {:name => "forwarding", :script => $ipv4_forwarding_enable},
      {:name => "cp_content", :script => $cp_content},
      {:name => "tools_install", :script => $tools_install},
      {:name => "p4c_install", :script => $p4c_install},
      {:name => "p4_build", :script => $p4_build},
      {:name => "nikss_build", :script => $nikss_build},
      {:name => "agent_build", :script => $agent_build},
    ]
  }
]

Vagrant.configure("2") do |config|
  servers.each do |machine|
    config.vm.define machine[:hostname] do |node|
      node.vm.box = machine[:box]
      node.vm.hostname = machine[:hostname]
      machine[:ip].each do |net|
        node.vm.network "private_network", ip: net[:addr], virtualbox__intnet: net[:name], netmask: "24"
      end
      node.vm.provider "virtualbox" do |vb|
        vb.memory = machine[:ram]
        vb.cpus = machine[:cpu]
      end
      machine[:scripts].each do |s|
        node.vm.provision s[:name], type: "shell", privileged: false, inline: s[:script]
      end
    end
  end
end
