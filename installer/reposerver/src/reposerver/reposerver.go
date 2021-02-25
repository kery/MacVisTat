package main

import (
	"fmt"
	"log"
	"net/http"
	"os"
	"sync"
	"text/tabwriter"
)

var mutex sync.Mutex
var fileStat map[string]uint32

type dirWithStat string

func (d dirWithStat) Open(name string) (http.File, error) {
	file, err := http.Dir(d).Open(name)
	if err == nil {
		mutex.Lock()
		fileStat[name] += 1
		mutex.Unlock()
	}
	return file, err
}

func userReportHandler(w http.ResponseWriter, r *http.Request) {
	hostName := r.PostFormValue("host")
	productType := r.PostFormValue("pt")
	version := r.PostFormValue("ver")

	file, err := os.OpenFile("reposerver.log", os.O_WRONLY|os.O_APPEND|os.O_CREATE, 0644)
	if err == nil {
		defer file.Close()
		log.SetOutput(file)
		log.Printf("%s started the client (%s, %s)", hostName, productType, version)
	}
}

func fileStatHandler(w http.ResponseWriter, r *http.Request) {
	tw := tabwriter.NewWriter(w, 0, 0, 4, ' ', 0)

	mutex.Lock()
	for name, count := range fileStat {
		fmt.Fprintf(tw, "%s: \t%d\n", name, count)
	}
	mutex.Unlock()

	tw.Flush()
}

func main() {
	fileStat = make(map[string]uint32)

	fs := http.FileServer(dirWithStat("."))
	http.Handle("/", fs)
	http.HandleFunc("/report", userReportHandler)
	http.HandleFunc("/fstat", fileStatHandler)

	http.ListenAndServe(":4099", nil)
}
