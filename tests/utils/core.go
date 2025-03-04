package utils

import (
	"errors"
	"strings"
)

type Core struct {
	machine *Machine
}

func NewCore(machine *Machine) *Core {
	core := Core{machine}
	return &core
}

func (core *Core) Install() error {
	result, err := core.machine.Command("inst")
	if err != nil {
		return err
	}
	if result != "" {
		return errors.New("Install command printed output when not expected")
	}

	dmesg, err := core.machine.ReadDmesg()
	if err != nil {
		return err
	}
	if !strings.Contains(dmesg, "Init switch_os kernel module") {
		return errors.New("Kernel module did not init")
	}

	return nil
}
