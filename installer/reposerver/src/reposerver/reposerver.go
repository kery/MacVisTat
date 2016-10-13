package main

import (
    "net/http"
    "log"
)

func userReportHandler(w http.ResponseWriter, r *http.Request) {
    hostName := r.PostFormValue("host")
    log.Printf("%s started the client", hostName)
}

func main() {
    fs := http.FileServer(http.Dir("."))
    http.Handle("/", fs)
    http.HandleFunc("/report", userReportHandler)

    http.ListenAndServe(":4099", nil)
}
