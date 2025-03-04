package utils

import (
	"fmt"

	"golang.org/x/crypto/ssh"
)

type Communicator struct {
	client *ssh.Client
	ip     string
}

func getMachineSshAddress(ip string) string {
	return fmt.Sprintf("%v:22", ip)
}

func NewCommunicator(network *Network) (*Communicator, error) {
	config := &ssh.ClientConfig{
		User:            "root",
		HostKeyCallback: ssh.InsecureIgnoreHostKey(),
	}

	err := network.WaitForMachineConnection()
	if err != nil {
		return nil, err
	}

	client, err := ssh.Dial("tcp", getMachineSshAddress(network.GetMachineIP()), config)
	if err != nil {
		return nil, err
	}

	comm := Communicator{client, network.GetMachineIP()}
	return &comm, nil
}

func (comm *Communicator) Destroy() {
	comm.client.Close()
}

func (comm *Communicator) Command(cmd string) (string, error) {
	session, err := comm.client.NewSession()
	if err != nil {
		return "", err
	}
	defer session.Close()

	result, err := session.Output(cmd)
	if err != nil {
		return "", err
	}

	return string(result), nil
}

func (comm *Communicator) CommandAsync(cmd string) error {
	session, err := comm.client.NewSession()
	if err != nil {
		return err
	}
	defer session.Close()

	return session.Start(cmd)
}
