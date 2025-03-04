package switch_os_test

import (
	"testing"

	. "github.com/onsi/ginkgo/v2"
	. "github.com/onsi/gomega"

	"libvirt.org/go/libvirt"
)

var conn *libvirt.Connect

func TestSwitchOS(t *testing.T) {
	RegisterFailHandler(Fail)
	RunSpecs(t, "Tests Suite")
}

var _ = BeforeSuite(func() {
	var err error
	conn, err = libvirt.NewConnect("qemu:///system")
	Expect(err).To(BeNil())
	DeferCleanup(conn.Close)
})
