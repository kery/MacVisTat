package main

import (
	"fmt"
	"log"
	"net/http"
	"os"
	"sort"
	"sync"
	"text/tabwriter"
)

var mutex sync.Mutex
var fileStat map[string]uint32

type dirWithStat string

func (d dirWithStat) Open(name string) (http.File, error) {
	file, err := http.Dir(d).Open(name)
	if err != nil {
		return nil, err
	}
	fi, err := file.Stat()
	if err != nil {
		return nil, err
	}
	if fi.IsDir() {
		return nil, os.ErrPermission
	}
	mutex.Lock()
	fileStat[name] += 1
	mutex.Unlock()
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
	if n := len(fileStat); n > 0 {
		keys := make([]string, 0, n)
		for name := range fileStat {
			keys = append(keys, name)
		}
		sort.Strings(keys)
		for _, name := range keys {
			fmt.Fprintf(tw, "%s \t%d\n", name, fileStat[name])
		}
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
