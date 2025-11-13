package main

import (
    "encoding/json"
    "fmt"
    "io"
    "os"
)

type Message struct {
    Id           string                 `json:"Id"`
    Status       string                 `json:"status"`
    Recipients   string                 `json:"status"`
    AdaptiveCard json.RawMessage	    `json:"AdaptiveCard"`
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

    var data map[string]interface{}

    if err := json.NewDecoder(os.Stdin).Decode(&data); err != nil {
        fmt.Fprintf(os.Stderr, "parse error: %v\n", err)
        os.Exit(1)
    }

    if status, ok := data["status"].(string); ok && status == "OPEN" {
        data["status"] = "CLOSE"
    }

    if err := json.NewEncoder(os.Stdout).Encode(data); err != nil {
        fmt.Fprintf(os.Stderr, "encode error: %v\n", err)
        os.Exit(1)
    }

}

