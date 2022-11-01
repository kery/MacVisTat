package main

import (
	"fmt"
	"io"
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

func usageReportHandler(w http.ResponseWriter, r *http.Request) {
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

func uploadHandler(w http.ResponseWriter, r *http.Request) {
	if err := r.ParseMultipartForm(1024 * 1024 * 10); err != nil {
		w.Write([]byte(err.Error()))
		return
	}
	file, header, err := r.FormFile("file")
	if err != nil {
		w.Write([]byte(err.Error()))
		return
	}
	defer file.Close()

	dstFile, err := os.Create(header.Filename)
	if err != nil {
		w.Write([]byte(err.Error()))
		return
	}
	defer dstFile.Close()

	if _, err = io.Copy(dstFile, file); err != nil {
		w.Write([]byte(err.Error()))
	}
}

func main() {
	fileStat = make(map[string]uint32)

	fs := http.FileServer(dirWithStat("."))
	http.Handle("/", fs)
	http.Handle("/plugins/", http.StripPrefix("/plugins/", http.FileServer(http.Dir("./plugins/lua/"))))
	http.HandleFunc("/report", usageReportHandler)
	http.HandleFunc("/fstat", fileStatHandler)
	http.HandleFunc("/upload", uploadHandler)

	http.ListenAndServe(":4099", nil)
}
