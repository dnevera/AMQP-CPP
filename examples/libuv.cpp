/**
 *  LibUV.cpp
 * 
 *  Test program to check AMQP functionality based on LibUV
 * 
 *  @author Emiel Bruijntjes <emiel.bruijntjes@copernica.com>
 *  @copyright 2015 - 2017 Copernica BV
 */

/**
 *  Dependencies
 */
#include <uv.h>
#include <amqpcpp.h>
#include <amqpcpp/libuv.h>
#include <future>
#include <thread>

/**
 *  Custom handler
 */
class MyHandler : public AMQP::LibUvHandler
{
private:
    /**
     *  Method that is called when a connection error occurs
     *  @param  connection
     *  @param  message
     */
    virtual void onError(AMQP::TcpConnection *connection, const char *message) override
    {
      std::cout << "error: " << message << std::endl;
    }

    /**
     *  Method that is called when the TCP connection ends up in a connected state
     *  @param  connection  The TCP connection
     */
    virtual void onConnected(AMQP::TcpConnection *connection) override
    {
      std::cout << "connected" << std::endl;
    }

public:
    /**
     *  Constructor
     *  @param  uv_loop
     */
    MyHandler(uv_loop_t *loop) : AMQP::LibUvHandler(loop) {}

    /**
     *  Destructor
     */
    virtual ~MyHandler() = default;
};

void timer_callback(uv_timer_t *handle){
  std::cout << "monitor ping ... " << std::endl;
}

void main_loop() {
  uv_async_t async;

  /* Main thread will run default loop */
  uv_loop_t *main_loop = uv_default_loop();
  uv_timer_t timer_req;
  uv_timer_init(main_loop, &timer_req);

  /* Timer callback needs async so it knows where to send messages */
  timer_req.data = &async;
  uv_timer_start(&timer_req, timer_callback, 0, 2000);

  std::cout << "Starting main loop" << std::endl;
  uv_run(main_loop, UV_RUN_DEFAULT);

}

/**
 *  Main program
 *  @return int
 */
int main()
{
  std::promise<std::string> promise_message;

//  std::thread thread([&promise_message]{
//      std::cout << "promise_message waiting  : " << std::endl;
//      std::cout << "promise_message recieved :  "<<promise_message.get_future().get() << std::endl;
//  });
//
//  thread.detach();


  uv_loop_t* loop = uv_loop_new();

  // handler for libev
  MyHandler handler(loop);

  // make a connection
  AMQP::TcpConnection connection(&handler, AMQP::Address("amqp://guest:guest@localhost/"));

  // we need a channel too
  AMQP::TcpChannel channel(&connection);

  channel

          .declareExchange("amq.topic", AMQP::topic, AMQP::durable)

          .onSuccess([]{
              std::cerr << "declared exchange saccess: " << std::endl;

          })
          .onError([](const char *message){
              std::cerr << "declared exchange error" << message << std::endl;

          });

  std::cout << "1... " << std::endl;

  // create a queue
  channel

          .declareQueue("capy-test", AMQP::durable)

          .onSuccess([&connection](const std::string &name, uint32_t messagecount, uint32_t consumercount) {

              // report the name of the temporary queue
              std::cout << "declared queue " << name << std::endl;
          })

          .onError([](const char *message){

              std::cerr << "declared queue error" << message << std::endl;

          });


  std::cout << "2... " << std::endl;

  channel

          .bindQueue("amq.topic", "capy-test", "echo.ping")

          .onSuccess([](){

              std::cout << "bind operation succeed..." << std::endl;

          })

          .onError([](const char *message) {

              std::cout << "bind operation failed: " << message << std::endl;
          });


  std::cout << "3... " << std::endl;

  channel

          .consume("capy-test")

          .onReceived([&channel](
                  const AMQP::Message &message,
                  uint64_t deliveryTag,
                  bool redelivered){

              std::cout << "message received: " << message.body() << " | "<< message.bodySize() << std::endl;

              // acknowledge the message
              channel.ack(deliveryTag);

              //promise_message.set_value(message.body());

          })

          .onSuccess( [](const std::string &consumertag) {

              std::cout << "consume operation started: " << consumertag << std::endl;
          })

          .onError([](const char *message) {

              std::cout << "consume operation failed: " << message << std::endl;
          });

  std::cout << "4... " << std::endl;

  std::thread thread_loop([loop] {
      uv_run(loop, UV_RUN_DEFAULT);
  });

  std::cout << "5... " << std::endl;

  thread_loop.detach();

  std::cout << "6... " << std::endl;

  main_loop();

  std::cout << "7... " << std::endl;

  // done
  return 0;
}

