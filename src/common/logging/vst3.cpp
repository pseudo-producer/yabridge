// yabridge: a Wine VST bridge
// Copyright (C) 2020  Robbert van der Helm
//
// This program is free software: you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with this program.  If not, see <https://www.gnu.org/licenses/>.

#include "vst3.h"

#include "src/common/serialization/vst3.h"

Vst3Logger::Vst3Logger(Logger& generic_logger) : logger(generic_logger) {}

void Vst3Logger::log_unknown_interface(
    const std::string& where,
    const std::optional<Steinberg::FUID>& uid) {
    if (BOOST_UNLIKELY(logger.verbosity >= Logger::Verbosity::most_events)) {
        char uid_string[128] = "<invalid_pointer>";
        if (uid) {
            uid->print(uid_string, Steinberg::FUID::UIDPrintStyle::kCLASS_UID);
        }

        std::ostringstream message;
        message << "[unknown interface] " << where << ": " << uid_string;

        log(message.str());
    }
}

void Vst3Logger::log_request(bool is_host_vst, const YaComponent::Construct&) {
    log_request_base(is_host_vst, [](auto& message) {
        // TODO: Log the cid in some readable way, if possible
        message << "IPluginFactory::createComponent(cid, IComponent::iid, "
                   "&obj)";
    });
}

void Vst3Logger::log_request(bool is_host_vst,
                             const YaComponent::Destruct& request) {
    log_request_base(is_host_vst, [&](auto& message) {
        message << "<IComponent* #" << request.instance_id
                << ">::~IComponent()";
    });
}

void Vst3Logger::log_request(bool is_host_vst,
                             const YaComponent::Initialize& request) {
    log_request_base(is_host_vst, [&](auto& message) {
        message << "<IComponent* #" << request.instance_id << ">::initialize(";
        if (request.host_application_context_args) {
            message << "IHostApplication*";
        } else {
            message << "nullptr";
        }
        message << ")";
    });
}

void Vst3Logger::log_request(bool is_host_vst,
                             const YaComponent::Terminate& request) {
    log_request_base(is_host_vst, [&](auto& message) {
        message << "<IComponent* #" << request.instance_id << ">::terminate()";
    });
}

void Vst3Logger::log_request(bool is_host_vst,
                             const YaPluginFactory::Construct&) {
    log_request_base(is_host_vst,
                     [](auto& message) { message << "GetPluginFactory()"; });
}

void Vst3Logger::log_request(bool is_host_vst, const WantsConfiguration&) {
    log_request_base(is_host_vst, [](auto& message) {
        message << "Requesting <Configuration>";
    });
}

void Vst3Logger::log_response(bool is_host_vst, const Ack&) {
    log_response_base(is_host_vst, [&](auto& message) { message << "ACK"; });
}

void Vst3Logger::log_response(
    bool is_host_vst,
    const std::variant<YaComponent::ConstructArgs, UniversalTResult>& result) {
    log_response_base(is_host_vst, [&](auto& message) {
        std::visit(overload{[&](const YaComponent::ConstructArgs& args) {
                                message << "<IComponent* #" << args.instance_id
                                        << ">";
                            },
                            [&](const UniversalTResult& code) {
                                message << code.string();
                            }},
                   result);
    });
}

void Vst3Logger::log_response(bool is_host_vst,
                              const YaPluginFactory::ConstructArgs& args) {
    log_response_base(is_host_vst, [&](auto& message) {
        message << "<IPluginFactory*> with " << args.num_classes
                << " registered classes";
    });
}

void Vst3Logger::log_response(bool is_host_vst, const Configuration&) {
    log_response_base(is_host_vst,
                      [](auto& message) { message << "<Configuration"; });
}
