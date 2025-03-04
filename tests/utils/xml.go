package utils

import (
	"errors"
	"path/filepath"
	"runtime"
	"strings"
	"text/template"
)

func GetSwitchOsDirPath() (string, error) {
	_, filename, _, ok := runtime.Caller(0)
	if !ok {
		return "", errors.New("Unable to get the current filename")
	}
	utilsDir := filepath.Dir(filename)
	testsDir := filepath.Dir(utilsDir)
	return filepath.Dir(testsDir), nil
}

func FormatStr(str string, values map[string]any) (string, error) {
	templ := template.Must(template.New("str").Parse(str))
	builder := &strings.Builder{}
	err := templ.Execute(builder, values)
	if err != nil {
		return "", err
	}

	return builder.String(), nil
}
