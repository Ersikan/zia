#include "api/http/request.hpp"
#include "api/initializer.hpp"
#include "api/mediator.hpp"
#include "router.hpp"
#include "utils.hpp"

extern "C" std::unique_ptr<zia::api::IModule>
load_module(zia::api::IZiaInitializer &initializer)
{
    auto router = std::make_unique<zia::router::Router>();

    initializer.registerConsumer(
        zia::api::event_descriptor<zia::api::http::NewHTTPRequest>,
        [&router](zia::api::IZiaMediator &mediator, std::unique_ptr<zia::api::IEvent> e) {
            auto event = zia::dynamic_unique_ptr_cast<zia::api::http::NewHTTPRequest>(
                std::move(e));
            auto route_result = router->routeRequest(event->getRequest().route);

            if (!route_result.has_value()) {
                zia::api::http::HTTPResponse not_found{404, "Not found", {}, ""};
                mediator.emit(event->createResponse(not_found));
            } else {
                std::unique_ptr<zia::api::IEvent> route_request =
                    std::make_unique<zia::router::RouteRequestEvent>(
                        route_result->handler, route_result->route, std::move(event));
                mediator.emit(std::move(route_request));
            }
        });

    return router;
}