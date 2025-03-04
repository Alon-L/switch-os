package switch_os_test

import (
	"switch_os/utils"
	"time"

	. "github.com/onsi/ginkgo/v2"
	. "github.com/onsi/gomega"
)

func EnsureMachineIsRunning(machine *utils.Machine) {
	GinkgoHelper()
	is_running, err := machine.IsRunning()
	Expect(err).NotTo(HaveOccurred())
	Expect(is_running).To(BeTrue())
}

var _ = Describe("SwitchOS", func() {
	var machine *utils.Machine

	BeforeEach(func() {
		var err error
		machine, err = utils.CreateMachine(conn)
		Expect(err).NotTo(HaveOccurred())
		DeferCleanup(machine.Destroy)

		EnsureMachineIsRunning(machine)
	})

	Describe("VM functionality", func() {
		It("should be running", func() {
			// Make sure the VM does not crash on boot.
			EnsureMachineIsRunning(machine)
			time.Sleep(10 * time.Second)
			EnsureMachineIsRunning(machine)
		})

		It("should be able to run commands", func() {
			result, err := machine.Command("echo HELLO")
			Expect(err).NotTo(HaveOccurred())
			Expect(result).To(Equal("HELLO\n"))
		})
	})

	Describe("Core logic", func() {
		var core *utils.Core

		BeforeEach(func() {
			machine.ClearDmesg()
			core = utils.NewCore(machine)
		})

		It("should install kernel module", func() {
			err := core.Install()
			Expect(err).NotTo(HaveOccurred())
		})
	})
})
