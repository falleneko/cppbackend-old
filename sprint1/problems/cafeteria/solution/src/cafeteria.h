#pragma once
#ifdef _WIN32
#include <sdkddkver.h>
#endif

#include <boost/asio/io_context.hpp>
#include <boost/asio/steady_timer.hpp>
#include <boost/asio/strand.hpp>
#include <memory>

#include "hotdog.h"
#include "result.h"

namespace net = boost::asio;

// Функция-обработчик операции приготовления хот-дога
using HotDogHandler = std::function<void(Result<HotDog> hot_dog)>;

class Order : public std::enable_shared_from_this<Order> {
public:
    Order(net::io_context& io, int id, std::shared_ptr<Sausage> sausage, std::shared_ptr<Bread> bread, HotDogHandler handler, std::shared_ptr<GasCooker> gas_cooker)
        : io_{io}
        , id_{id}
        , sausage_{std::move(sausage)}
        , bread_{std::move(bread)}
        , handler_{std::move(handler)}
        , gas_cooker_{std::move(gas_cooker)} {
    }

    // Запускает асинхронное выполнение заказа
    void Execute() {
        net::post(io_, [self = shared_from_this()]() {
            try {
                self->bread_->StartBake(*self->gas_cooker_, [self] {
                    net::steady_timer timer(self->io_, HotDog::MIN_BREAD_COOK_DURATION);
                    timer.wait();
                    self->bread_->StopBaking();
                    if (self->IsReadyToPack()) {
                        self->Pack();
                    }
                });
            } catch (...) {
                self->Deliver(std::current_exception());
            }
        });
        net::post(io_, [self = shared_from_this()]() {
            try {
                self->sausage_->StartFry(*self->gas_cooker_, [self] {
                    net::steady_timer timer(self->io_, HotDog::MIN_SAUSAGE_COOK_DURATION);
                    timer.wait();
                    self->sausage_->StopFry();
                    if (self->IsReadyToPack()) {
                        self->Pack();
                    }
                });
            } catch (...) {
                self->Deliver(std::current_exception());
            }
        });
    }

private:
    [[nodiscard]] bool IsReadyToPack() const {
        return bread_->IsCooked() && sausage_->IsCooked();
    }

    void Pack() {
        if (delivered_) {
            Deliver(std::make_exception_ptr(std::logic_error("Hot dog has already been delivered")));
            return;
        }
        if (!IsReadyToPack()) {
            Deliver(std::make_exception_ptr(std::logic_error("Hot dog is not ready to pack")));
            return;
        }
        Deliver({});
    }

    void Deliver(std::exception_ptr error) {
        delivered_ = true;
        if (error) {
            handler_(Result<HotDog>{std::move(error)});
        } else {
            handler_(Result<HotDog>{HotDog{id_, sausage_, bread_}});
        }
    }

    net::io_context& io_;
    int id_;
    std::shared_ptr<Sausage> sausage_;
    std::shared_ptr<Bread> bread_;
    HotDogHandler handler_;
    std::shared_ptr<GasCooker> gas_cooker_;
    bool delivered_ = false;
    static const int MAX_SAUSAGE_COOK_DURATION_MS = 1500;
    static const int MAX_BREAD_COOK_DURATION_MS = 1000;
};

// Класс "Кафетерий". Готовит хот-доги
class Cafeteria {
public:
    explicit Cafeteria(net::io_context& io)
        : io_{io},
          strand_{net::make_strand(io_)} {
    }

    // Асинхронно готовит хот-дог и вызывает handler, как только хот-дог будет готов.
    // Этот метод может быть вызван из произвольного потока
    void OrderHotDog(HotDogHandler handler) {
        net::post(strand_, [handler = std::move(handler), this]() {
            int id = NextHotDogId();
            std::shared_ptr<Sausage> sausage = store_.GetSausage();
            std::shared_ptr<Bread> bread = store_.GetBread();
            
            net::post(io_, [this, id, handler = std::move(handler), sausage = std::move(sausage), bread = std::move(bread)]() {
                auto order = std::make_shared<Order>(
                    io_,
                    id,
                    sausage,
                    bread,
                    std::move(handler),
                    gas_cooker_
                );
                order->Execute();
            });
        });
    }

private:

    int NextHotDogId() {
        return ++hot_dog_id_;
    }

    int hot_dog_id_ = 0;
    net::io_context& io_;
    net::strand<net::io_context::executor_type> strand_;
    // Используется для создания ингредиентов хот-дога
    Store store_;
    // Газовая плита. По условию задачи в кафетерии есть только одна газовая плита на 8 горелок
    // Используйте её для приготовления ингредиентов хот-дога.
    // Плита создаётся с помощью make_shared, так как GasCooker унаследован от
    // enable_shared_from_this.
    std::shared_ptr<GasCooker> gas_cooker_ = std::make_shared<GasCooker>(io_);
};
