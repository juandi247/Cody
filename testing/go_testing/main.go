package main

import (
	"fmt"
	"log"
	"net"
	"net/http"
	"strconv"
	"sync"
	"time"
	//"time"
)

func main() {
	fmt.Println("starting goroutines")

	var wg sync.WaitGroup

	for i := 1; i < 2; i++ {
		wg.Add(1)
		// go test_connection(i, &wg)
		go httpRequest(i, &wg)
	}
	wg.Wait()
}


func httpRequest(index int, wg *sync.WaitGroup){
	defer func(){
		wg.Done()
	}()

	fmt.Printf("vamos a tirar una request goroutine: %d \n" , index)
	// time.Sleep(time.Second*2)
_, err:= http.Get("http://localhost:8080/index.html")

if err!=nil{
	log.Fatal("error en la request", err)
	return
}

}


func test_connection(index int, wg *sync.WaitGroup) {

	conn, err := net.Dial("tcp", "localhost:8090")
	if err != nil {

		fmt.Printf("fallo algo %v", err)
		return
	}

	defer func(){
		conn.Close()
		wg.Done()
	}()
	fmt.Printf("DIAl para la conex: %d done, vamos a escribir \n", index)

	message := " message from goroutine: " + strconv.Itoa(index) + "\n"
	//	time.Sleep(time.Second * 2)
	bytesWritten, err := conn.Write([]byte(message))

	
	if err != nil {
		fmt.Errorf("error", err)
	}
	if bytesWritten < len(message) {

		fmt.Errorf("not sendet all the data")

	}
	time.Sleep(time.Second * 2)
	secondMessage := "Segunda Parte de Goroutine: " + strconv.Itoa(index) + "\n"
	bytesWritten, err = conn.Write([]byte(secondMessage))

	fmt.Println("Written goroutine ", index)
}
