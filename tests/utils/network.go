package utils

import (
	"errors"
	"time"

	"libvirt.org/go/libvirt"
)

var networkSubnet string = "10.10.10.1"
var machineIP string = "10.10.10.50"

var networkXmlTemplate string = `
<network>
  <name>switch_os_network</name>
  <bridge name='virbr_switchos'/>
  <forward mode='nat'/>
  <ip address='{{ .networkSubnet }}' netmask='255.255.255.0'>
    <dhcp>
      <host mac='52:54:00:12:34:56' name='switch_os' ip='{{ .machineIP }}'/>
    </dhcp>
  </ip>
</network>
`

type Network struct {
	network *libvirt.Network
}

func NewNetwork(conn *libvirt.Connect) (*Network, error) {
	networkXml, err := FormatStr(networkXmlTemplate, map[string]any{
		"networkSubnet": networkSubnet,
		"machineIP":     machineIP,
	})
	if err != nil {
		return nil, err
	}

	libvirtNetwork, err := conn.NetworkCreateXML(networkXml)
	if err != nil {
		return nil, err
	}

	network := Network{network: libvirtNetwork}
	return &network, nil
}

func (network *Network) Destroy() {
	network.network.Destroy()
}

func (network *Network) WaitForMachineConnection() error {
	for range 30 {
		dhcpLeases, err := network.network.GetDHCPLeases()
		if err != nil {
			return err
		}

		isMachineConnected := (len(dhcpLeases) > 0)
		if isMachineConnected {
			// Let the machine assign the IP to its interface.
			time.Sleep(1 * time.Second)
			return nil
		}

		time.Sleep(1 * time.Second)
	}

	return errors.New("Machine failed to connect during the allowed timeout")
}

func (network *Network) GetMachineIP() string {
	return machineIP
}
