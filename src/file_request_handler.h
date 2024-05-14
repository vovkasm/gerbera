/*MT*

    MediaTomb - http://www.mediatomb.cc/

    file_request_handler.h - this file is part of MediaTomb.

    Copyright (C) 2005 Gena Batyan <bgeradz@mediatomb.cc>,
                       Sergey 'Jin' Bostandzhyan <jin@mediatomb.cc>

    Copyright (C) 2006-2010 Gena Batyan <bgeradz@mediatomb.cc>,
                            Sergey 'Jin' Bostandzhyan <jin@mediatomb.cc>,
                            Leonhard Wimmer <leo@mediatomb.cc>

    Copyright (C) 2016-2024 Gerbera Contributors

    MediaTomb is free software; you can redistribute it and/or modify
    it under the terms of the GNU General Public License version 2
    as published by the Free Software Foundation.

    MediaTomb is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    version 2 along with MediaTomb; if not, write to the Free Software
    Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA 02110-1301, USA.

    $Id$
*/

/// \file file_request_handler.h
/// \brief Definition of the FileRequestHandler class.
#ifndef __FILE_REQUEST_HANDLER_H__
#define __FILE_REQUEST_HANDLER_H__

#include "request_handler.h"

#include <memory>

#include "upnp/xml_builder.h"

class CdsResource;
class MetadataHandler;
class MetadataService;

class FileRequestHandler : public RequestHandler {

public:
    explicit FileRequestHandler(const std::shared_ptr<ContentManager>& content, const std::shared_ptr<UpnpXMLBuilder>& xmlBuilder, std::shared_ptr<MetadataService> metadataService);

    const struct ClientInfo* getInfo(const char* filename, UpnpFileInfo* info) override;
    std::unique_ptr<IOHandler> open(const char* filename, const std::shared_ptr<Quirks>& quirks, enum UpnpOpenFileMode mode) override;

private:
    static std::size_t parseResourceInfo(const std::map<std::string, std::string>& params);
    std::shared_ptr<MetadataHandler> getResourceMetadataHandler(std::shared_ptr<CdsObject>& obj, std::shared_ptr<CdsResource>& resource) const;

    std::shared_ptr<MetadataService> metadataService;
};

#endif // __FILE_REQUEST_HANDLER_H__
