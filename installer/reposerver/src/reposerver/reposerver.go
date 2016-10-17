package main

import (
    "net/http"
    "log"
    "os"
)

func userReportHandler(w http.ResponseWriter, r *http.Request) {
    hostName := r.PostFormValue("host")
    productType := r.PostFormValue("pt")
    version := r.PostFormValue("ver")

    file, err := os.OpenFile("reposerver.log", os.O_WRONLY | os.O_APPEND | os.O_CREATE, 0644)
    if err == nil {
        defer file.Close()
        log.SetOutput(file)
        log.Printf("%s started the client (%s, %s)", hostName, productType, version)
    }
}

func main() {
    fs := http.FileServer(http.Dir("."))
    http.Handle("/", fs)
    http.HandleFunc("/report", userReportHandler)

    http.ListenAndServe(":4099", nil)
}
