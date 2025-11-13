package main

import (
	"encoding/json"
	"flag"
	"fmt"
	"github.com/google/uuid"
	"io"
	"os"
)

// github.com/spf13/cobra apparently worth looking into

type Message struct {
	Id           string          `json:"Id"`
	Status       string          `json:"Status"`
	Recipients   []string        `json:"Recipients"`
	AdaptiveCard json.RawMessage `json:"AdaptiveCard"`
}

type StatusFlag string

const (
	StatusUnchanged StatusFlag = "UNCHANGED"
	StatusOpen      StatusFlag = "OPEN"
	StatusClose     StatusFlag = "CLOSE"
)

func (s *StatusFlag) String() string {
	return string(*s)
}

func (s *StatusFlag) Set(v string) error {
	switch v {
	case "open":
		*s = StatusOpen
	case "close":
		*s = StatusClose
	case "unchanged":
		*s = StatusUnchanged
	default:
		return fmt.Errorf("invalid status: %s (must be: open, close, unchanged)", v)
	}
	return nil
}

func getNestedString(m map[string]interface{}, keys ...string) (string, bool) {
	var current interface{} = m
	for _, key := range keys {
		if obj, ok := current.(map[string]interface{}); ok {
			current = obj[key]
		} else {
			return "", false
		}
	}
	if s, ok := current.(string); ok {
		return s, true
	}
	return "", false
}

func main() {
	var input = flag.String("input", "", "input file path")
	var newMsg = flag.Bool("new", false, "generate & use a new UUID instead of the existing")

	var status StatusFlag
	flag.Var(&status, "status", "set status(open|close|unchanged)")

	flag.Parse()

	var reader io.Reader
	if *input != "" {
		f, err := os.Open(*input)
		if err != nil {
			fmt.Fprintf(os.Stderr, "error opening file: %v\n", err)
			os.Exit(1)
		}
		defer f.Close()
		reader = f
	} else {
		reader = os.Stdin
	}

	var msg Message
	err := json.NewDecoder(reader).Decode(&msg)
	if err != nil {
		fmt.Fprintf(os.Stderr, "json decode error error: '%s'\n", err)
		os.Exit(1)
	}

	if status != "" {
		msg.Status = string(status)
	}

	if *newMsg {
		msg.Id = uuid.New().String()
	}

	if err := json.NewEncoder(os.Stdout).Encode(msg); err != nil {
		fmt.Fprintf(os.Stderr, "encode error: %v\n", err)
		os.Exit(1)
	}

	fmt.Fprintf(os.Stderr, "All done! Nice la!")

}
