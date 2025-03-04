package utils

import (
	"os"
	"strings"
)

var logPath string = "/var/log/switch-os.log"

type Logger struct {
	path string
}

func NewLogger() *Logger {
	logger := Logger{path: logPath}
	return &logger
}

func (logger *Logger) read() (string, error) {
	data, err := os.ReadFile(logger.path)
	if err != nil {
		return "", err
	}

	return string(data), nil
}

func (logger *Logger) IsError() (bool, error) {
	log, err := logger.read()
	if err != nil {
		return false, err
	}

	isError := strings.Contains(log, "Error")
	return isError, nil
}

func (logger *Logger) IsCoreComplete() (bool, error) {
	log, err := logger.read()
	if err != nil {
		return false, err
	}

	isCoreComplete := strings.Contains(log, "Waking up kernel")
	return isCoreComplete, nil
}
