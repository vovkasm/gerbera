#include <gtest/gtest.h>

#include "cds_objects.h"
#include "common.h"
#include "metadata/metadata_handler.h"
#include "transcoding/transcoding.h"
#include "upnp_xml.h"

#include "../mock/config_mock.h"
#include "../mock/database_mock.h"

using ::testing::_;
using ::testing::Return;

class UpnpXmlTest : public ::testing::Test {

public:
    UpnpXmlTest() = default;
    ~UpnpXmlTest() override = default;

    void SetUp() override
    {
        config = std::make_shared<ConfigMock>();

        database = std::make_shared<DatabaseMock>(config);
        context = std::make_shared<Context>(config, nullptr, nullptr, database, nullptr, nullptr);

        std::string virtualDir = "http://server/";
        std::string presentationURl = "http://someurl/";
        subject = new UpnpXMLBuilder(context, virtualDir, presentationURl);
    }

    void TearDown() override
    {
        delete subject;
    }

    UpnpXMLBuilder* subject;
    std::shared_ptr<ConfigMock> config;
    std::shared_ptr<DatabaseMock> database;
    std::shared_ptr<Context> context;
};

TEST_F(UpnpXmlTest, RenderObjectContainer)
{
    // arrange
    pugi::xml_document didlLite;
    auto root = didlLite.append_child("DIDL-Lite");
    auto obj = std::make_shared<CdsContainer>();
    obj->setID(1);
    obj->setParentID(2);
    obj->setRestricted(false);
    obj->setTitle("Title");
    obj->setClass(UPNP_CLASS_MUSIC_ALBUM);
    obj->addMetaData(M_ALBUMARTIST, "Creator");
    obj->addMetaData(M_COMPOSER, "Composer");
    obj->addMetaData(M_CONDUCTOR, "Conductor");
    obj->addMetaData(M_ORCHESTRA, "Orchestra");
    obj->addMetaData(M_UPNP_DATE, "2001-01-01");
    obj->addMetaData(M_DATE, "2022-04-01T00:00:00");

    // albumArtURI
    auto resource = std::make_shared<CdsResource>(ContentHandler::CONTAINERART, CdsResource::Purpose::Thumbnail);
    resource->addAttribute(CdsResource::Attribute::PROTOCOLINFO, renderProtocolInfo("jpg"));
    resource->addAttribute(CdsResource::Attribute::RESOURCE_FILE, "/home/resource/cover.jpg");
    obj->addResource(resource);

    std::ostringstream expectedXml;
    expectedXml << "<DIDL-Lite>\n";
    expectedXml << "<container id=\"1\" parentID=\"2\" restricted=\"0\">\n";
    expectedXml << "<dc:title>Title</dc:title>\n";
    expectedXml << "<upnp:class>object.container.album.musicAlbum</upnp:class>\n";
    expectedXml << "<dc:creator>Creator</dc:creator>\n";
    expectedXml << "<dc:date>2022-04-01T00:00:00</dc:date>\n";
    expectedXml << "<upnp:albumArtist>Creator</upnp:albumArtist>\n";
    expectedXml << "<upnp:artist>Creator</upnp:artist>\n";
    expectedXml << "<upnp:composer>Composer</upnp:composer>\n";
    expectedXml << "<upnp:conductor>Conductor</upnp:conductor>\n";
    expectedXml << "<upnp:date>2001-01-01</upnp:date>\n";
    expectedXml << "<upnp:orchestra>Orchestra</upnp:orchestra>\n";
    expectedXml << "<upnp:albumArtURI>http://server/content/media/object_id/1/res_id/0</upnp:albumArtURI>\n";
    expectedXml << "</container>\n";
    expectedXml << "</DIDL-Lite>\n";

    // act
    subject->renderObject(obj, std::string::npos, root);

    // assert
    std::ostringstream buf;
    didlLite.print(buf, "", 0);
    std::string didlLiteXml = buf.str();
    EXPECT_STREQ(didlLiteXml.c_str(), expectedXml.str().c_str());
}

TEST_F(UpnpXmlTest, RenderObjectItem)
{
    // arrange
    pugi::xml_document didlLite;
    auto root = didlLite.append_child("DIDL-Lite");
    auto obj = std::make_shared<CdsItem>();
    obj->setID(1);
    obj->setParentID(2);
    obj->setRestricted(false);
    obj->setTitle("Title");
    obj->setClass(UPNP_CLASS_MUSIC_TRACK);
    obj->addMetaData(M_DESCRIPTION, "Description");
    obj->addMetaData(M_ALBUM, "Album");
    obj->addMetaData(M_TRACKNUMBER, "10");
    obj->addMetaData(M_DATE, "2022-04-01T00:00:00");

    std::ostringstream expectedXml;
    expectedXml << "<DIDL-Lite>\n";
    expectedXml << "<item id=\"1\" parentID=\"2\" restricted=\"0\">\n";
    expectedXml << "<dc:title>Title</dc:title>\n";
    expectedXml << "<upnp:class>object.item.audioItem.musicTrack</upnp:class>\n";
    expectedXml << "<dc:date>2022-04-01T00:00:00</dc:date>\n";
    expectedXml << "<dc:description>Description</dc:description>\n";
    expectedXml << "<upnp:album>Album</upnp:album>\n";
    expectedXml << "<upnp:originalTrackNumber>10</upnp:originalTrackNumber>\n";
    expectedXml << "</item>\n";
    expectedXml << "</DIDL-Lite>\n";

    EXPECT_CALL(*config, getOption(CFG_IMPORT_LIBOPTS_ENTRY_SEP))
        .WillRepeatedly(Return(" / "));
    log_warning(CFG_IMPORT_LIBOPTS_ENTRY_SEP);
    EXPECT_CALL(*config, getTranscodingProfileListOption(_))
        .WillRepeatedly(Return(std::make_shared<TranscodingProfileList>()));

    // act
    subject->renderObject(obj, std::string::npos, root);

    // assert
    std::ostringstream buf;
    didlLite.print(buf, "", 0);
    std::string didlLiteXml = buf.str();
    EXPECT_STREQ(didlLiteXml.c_str(), expectedXml.str().c_str());
}

TEST_F(UpnpXmlTest, RenderObjectItemWithResources)
{
    // arrange
    pugi::xml_document didlLite;
    auto root = didlLite.append_child("DIDL-Lite");
    auto obj = std::make_shared<CdsItem>();
    obj->setID(42);
    obj->setParentID(2);
    obj->setRestricted(false);
    obj->setTitle("Title");
    obj->setClass(UPNP_CLASS_MUSIC_TRACK);
    obj->addMetaData(M_DESCRIPTION, "Description");
    obj->addMetaData(M_ALBUM, "Album");
    obj->addMetaData(M_TRACKNUMBER, "7");
    obj->addMetaData(M_UPNP_DATE, "2002-01-01");
    obj->addMetaData(M_DATE, "2022-04-01T00:00:00");

    auto resource = std::make_shared<CdsResource>(ContentHandler::DEFAULT, CdsResource::Purpose::Content);
    resource->addAttribute(CdsResource::Attribute::PROTOCOLINFO, "http-get:*:audio/mpeg:*");
    resource->addAttribute(CdsResource::Attribute::BITRATE, "16044");
    resource->addAttribute(CdsResource::Attribute::DURATION, "123456");
    resource->addAttribute(CdsResource::Attribute::NRAUDIOCHANNELS, "2");
    resource->addAttribute(CdsResource::Attribute::SIZE, "4711");
    obj->addResource(resource);

    resource = std::make_shared<CdsResource>(ContentHandler::SUBTITLE, CdsResource::Purpose::Subtitle);
    std::string type = "srt";
    resource->addAttribute(CdsResource::Attribute::PROTOCOLINFO, renderProtocolInfo(type));
    resource->addAttribute(CdsResource::Attribute::RESOURCE_FILE, "/home/resource/subtitle.srt");
    resource->addParameter("type", type);
    obj->addResource(resource);

    resource = std::make_shared<CdsResource>(ContentHandler::FANART, CdsResource::Purpose::Thumbnail);
    resource->addAttribute(CdsResource::Attribute::PROTOCOLINFO, renderProtocolInfo("jpg"));
    resource->addAttribute(CdsResource::Attribute::RESOURCE_FILE, "/home/resource/cover.jpg");
    resource->addAttribute(CdsResource::Attribute::RESOLUTION, "200x200");
    obj->addResource(resource);

    std::ostringstream expectedXml;
    expectedXml << "<DIDL-Lite>\n";
    expectedXml << "<item id=\"42\" parentID=\"2\" restricted=\"0\">\n";
    expectedXml << "<dc:title>Title</dc:title>\n";
    expectedXml << "<upnp:class>object.item.audioItem.musicTrack</upnp:class>\n";
    expectedXml << "<dc:date>2022-04-01T00:00:00</dc:date>\n";
    expectedXml << "<dc:description>Description</dc:description>\n";
    expectedXml << "<upnp:album>Album</upnp:album>\n";
    expectedXml << "<upnp:date>2002-01-01</upnp:date>\n";
    expectedXml << "<upnp:originalTrackNumber>7</upnp:originalTrackNumber>\n";
    expectedXml << "<upnp:albumArtURI xmlns:dlna=\"urn:schemas-dlna-org:metadata-1-0\" dlna:profileID=\"JPEG_TN\">http://server/content/media/object_id/42/res_id/2</upnp:albumArtURI>\n";
    expectedXml << "<sec:CaptionInfoEx protocolInfo=\"http-get:*:srt:*\" sec:type=\"srt\">http://server/content/media/object_id/42/res_id/1/type/srt/ext/file.subtitle.srt</sec:CaptionInfoEx>\n";
    expectedXml << "<res size=\"4711\" duration=\"123456\" bitrate=\"16044\" nrAudioChannels=\"2\" protocolInfo=\"http-get:*:audio/mpeg:DLNA.ORG_PN=MP3;DLNA.ORG_OP=01;DLNA.ORG_CI=0;DLNA.ORG_FLAGS=01700000000000000000000000000000\">http://server/content/media/object_id/42/res_id/0/group/default/ext/file.mp3</res>\n";
    expectedXml << "<res protocolInfo=\"http-get:*:srt:DLNA.ORG_OP=01;DLNA.ORG_CI=0;DLNA.ORG_FLAGS=00d00000000000000000000000000000\">http://server/content/media/object_id/42/res_id/1/group/default/type/srt/ext/file.subtitle.srt</res>\n";
    expectedXml << "</item>\n";
    expectedXml << "</DIDL-Lite>\n";

    EXPECT_CALL(*config, getOption(CFG_IMPORT_LIBOPTS_ENTRY_SEP))
        .WillRepeatedly(Return(" / "));
    EXPECT_CALL(*config, getTranscodingProfileListOption(_))
        .WillRepeatedly(Return(std::make_shared<TranscodingProfileList>()));

    // act
    subject->renderObject(obj, std::string::npos, root);

    // assert
    std::ostringstream buf;
    didlLite.print(buf, "", 0);
    std::string didlLiteXml = buf.str();

    EXPECT_STREQ(didlLiteXml.c_str(), expectedXml.str().c_str());
}

TEST_F(UpnpXmlTest, CreatesEventPropertySet)
{
    auto result = UpnpXMLBuilder::createEventPropertySet();
    auto root = result->document_element();

    EXPECT_NE(root, nullptr);
    EXPECT_STREQ(root.name(), "e:propertyset");
    EXPECT_STREQ(root.attribute("xmlns:e").as_string(), "urn:schemas-upnp-org:event-1-0");
    EXPECT_NE(root.child("e:property"), nullptr);
}

TEST_F(UpnpXmlTest, CreateResponse)
{
    std::string actionName = "action";
    std::string serviceType = "urn:schemas-upnp-org:service:ContentDirectory:1";

    auto result = UpnpXMLBuilder::createResponse(actionName, serviceType);
    EXPECT_NE(result, nullptr);

    auto root = result->document_element();
    EXPECT_STREQ(root.name(), "u:actionResponse");
    EXPECT_STREQ(root.attribute("xmlns:u").value(), "urn:schemas-upnp-org:service:ContentDirectory:1");
}

TEST_F(UpnpXmlTest, FirstResourceRendersPureWhenExternalUrl)
{
    auto obj = std::make_shared<CdsItemExternalURL>();
    obj->setLocation("http://localhost/external/url");

    auto item = std::static_pointer_cast<CdsItem>(obj);

    std::string result = UpnpXMLBuilder::getFirstResourcePath(item);

    EXPECT_NE(result, "");
    EXPECT_STREQ(result.c_str(), "http://localhost/external/url");
}

TEST_F(UpnpXmlTest, FirstResourceAddsLocalResourceIdToExternalUrlWhenOnlineWithProxy)
{
    auto obj = std::make_shared<CdsItemExternalURL>();
    obj->setLocation("http://localhost/external/url");
    obj->setID(12345);
    obj->setFlag(OBJECT_FLAG_ONLINE_SERVICE);
    obj->setFlag(OBJECT_FLAG_PROXY_URL);

    auto item = std::static_pointer_cast<CdsItem>(obj);

    std::string result = UpnpXMLBuilder::getFirstResourcePath(item);

    EXPECT_NE(result, "");
    EXPECT_STREQ(result.c_str(), "content/online/object_id/12345/res_id/0");
}

TEST_F(UpnpXmlTest, FirstResourceAddsLocalResourceIdToItem)
{
    auto obj = std::make_shared<CdsItem>();
    obj->setLocation("local/content");
    obj->setID(12345);

    auto item = std::static_pointer_cast<CdsItem>(obj);

    std::string result = UpnpXMLBuilder::getFirstResourcePath(item);

    EXPECT_NE(result, "");
    EXPECT_STREQ(result.c_str(), "content/media/object_id/12345/res_id/0");
}
