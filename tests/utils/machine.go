package utils

import (
	"libvirt.org/go/libvirt"
)

var domainXmlTemplate string = `
<domain type='kvm'>
  <name>switch-os</name>
  <memory unit='GB'>2</memory>
  <vcpu>2</vcpu>
  <os>
    <type arch='x86_64' machine='pc-q35-7.2'>hvm</type>
    <loader readonly='yes' type='pflash'>/usr/share/OVMF/OVMF_CODE_4M.fd</loader>
    <nvram template='/usr/share/OVMF/OVMF_VARS_4M.fd' templateFormat='raw' format='raw'>/usr/share/OVMF/OVMF_VARS_4M.fd</nvram>
    <kernel>{{ .switchOsDirPath }}/tests/binaries/bzImage-6_6</kernel>
    <initrd>{{ .switchOsDirPath }}/tests/binaries/initramfs.cpio.gz</initrd>
    <cmdline>console=ttyS0 memmap=64M$1G,4K$4K</cmdline>
  </os>
  <devices>
    <emulator>/usr/bin/qemu-system-x86_64</emulator>
    <filesystem type='mount'>
      <source dir='{{ .switchOsDirPath }}/build'/>
      <target dir='qemu_root'/>
      <readonly/>
    </filesystem>
    <disk type='file'>
      <driver name='qemu' type='raw'/>
      <source file='{{ .switchOsDirPath }}/dump.img'/>
      <target dev='vdb' bus='virtio'/>
    </disk>
    <interface type='network'>
      <source network='switch_os_network'/>
      <mac address='52:54:00:12:34:56'/>
      <model type='virtio'/>
    </interface>
    <console type='pty'>
      <target type='serial' port='0'/>
    </console>
  </devices>
  <features>
    <kvm/>
    <acpi/>
  </features>
  <log file='{{ .logPath }}' level='debug'/>
</domain>
`

type Machine struct {
	domain  *libvirt.Domain
	network *Network
	comm    *Communicator
}

func CreateMachine(conn *libvirt.Connect) (_ *Machine, err error) {
	machine := Machine{}
	defer func() {
		if err != nil {
			if machine.domain != nil {
				machine.domain.Destroy()
			}
			if machine.network != nil {
				machine.network.Destroy()
			}
			if machine.comm != nil {
				machine.comm.Destroy()
			}
		}
	}()

	switchOsDirPath, err := GetSwitchOsDirPath()
	if err != nil {
		return nil, err
	}

	domainXml, err := FormatStr(domainXmlTemplate, map[string]any{
		"switchOsDirPath": switchOsDirPath,
		"logPath":         logPath,
	})
	if err != nil {
		return nil, err
	}

	network, err := NewNetwork(conn)
	if err != nil {
		return nil, err
	}
	machine.network = network

	domain, err := conn.DomainCreateXML(domainXml, libvirt.DOMAIN_NONE)
	if err != nil {
		return nil, err
	}
	machine.domain = domain

	comm, err := NewCommunicator(network)
	if err != nil {
		return nil, err
	}
	machine.comm = comm

	return &machine, nil
}

func (machine *Machine) Destroy() {
	machine.domain.Destroy()
	machine.network.Destroy()
	machine.comm.Destroy()
}

func (machine *Machine) IsRunning() (bool, error) {
	state, _, err := machine.domain.GetState()
	if err != nil {
		return false, err
	}

	is_running := (state == libvirt.DOMAIN_RUNNING)
	return is_running, nil
}

func (machine *Machine) Command(cmd string) (string, error) {
	return machine.comm.Command(cmd)
}

func (machine *Machine) CommandAsync(cmd string) error {
	return machine.comm.CommandAsync(cmd)
}

func (machine *Machine) ClearDmesg() error {
	return machine.CommandAsync("dmesg -C")
}

func (machine *Machine) ReadDmesg() (string, error) {
	return machine.Command("dmesg -c")
}
