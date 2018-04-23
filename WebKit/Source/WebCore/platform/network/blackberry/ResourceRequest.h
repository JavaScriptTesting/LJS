/*
 * Copyright (C) 2009, 2010 Research In Motion Limited. All rights reserved.
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with this library; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301  USA
 */

#ifndef ResourceRequest_h
#define ResourceRequest_h

#include "ResourceRequestBase.h"

namespace BlackBerry {
namespace Platform {
class NetworkRequest;
}
}

namespace WebCore {

class ResourceRequest : public ResourceRequestBase {
public:
    ResourceRequest(const String& url)
        : ResourceRequestBase(KURL(ParsedURLString, url), UseProtocolCachePolicy)
        , m_isXMLHTTPRequest(false)
        , m_mustHandleInternally(false)
        , m_isRequestedByPlugin(false)
        , m_forceDownload(false)
    {
    }

    ResourceRequest(const KURL& url)
        : ResourceRequestBase(url, UseProtocolCachePolicy)
        , m_isXMLHTTPRequest(false)
        , m_mustHandleInternally(false)
        , m_isRequestedByPlugin(false)
        , m_forceDownload(false)
    {
    }

    ResourceRequest(const KURL& url, const String& referrer, ResourceRequestCachePolicy policy = UseProtocolCachePolicy)
        : ResourceRequestBase(url, policy)
        , m_isXMLHTTPRequest(false)
        , m_mustHandleInternally(false)
        , m_isRequestedByPlugin(false)
        , m_forceDownload(false)
    {
        setHTTPReferrer(referrer);
    }

    ResourceRequest()
        : ResourceRequestBase(KURL(), UseProtocolCachePolicy)
        , m_isXMLHTTPRequest(false)
        , m_mustHandleInternally(false)
        , m_isRequestedByPlugin(false)
        , m_forceDownload(false)
    {
    }

    void setToken(const String& token) { m_token = token; }
    String token() const { return m_token; }

    // FIXME: For RIM Bug #452. The BlackBerry application wants the anchor text for a clicked hyperlink so as to
    // make an informed decision as to whether to allow the navigation. We should move this functionality into a
    // UI/Policy delegate.
    void setAnchorText(const String& anchorText) { m_anchorText = anchorText; }
    String anchorText() const { return m_anchorText; }

    void setOverrideContentType(const String& contentType) { m_overrideContentType = contentType; }
    String overrideContentType() const { return m_overrideContentType; }

    void setIsXMLHTTPRequest(bool isXMLHTTPRequest) { m_isXMLHTTPRequest = isXMLHTTPRequest; }
    bool isXMLHTTPRequest() const { return m_isXMLHTTPRequest; }

    void setIsRequestedByPlugin(bool isRequestedByPlugin) { m_isRequestedByPlugin = isRequestedByPlugin; }
    bool isRequestedByPlugin() const { return m_isRequestedByPlugin; }

    // Marks requests which must be handled by webkit even if LinksHandledExternally is set.
    void setMustHandleInternally(bool mustHandleInternally) { m_mustHandleInternally = mustHandleInternally; }
    bool mustHandleInternally() const { return m_mustHandleInternally; }

    void initializePlatformRequest(BlackBerry::Platform::NetworkRequest&, bool isInitial = false) const;
    void setForceDownload(bool forceDownload) { m_forceDownload = true; }
    bool forceDownload() const { return m_forceDownload; }

private:
    friend class ResourceRequestBase;

    String m_token;
    String m_anchorText;
    String m_overrideContentType;
    bool m_isXMLHTTPRequest;
    bool m_mustHandleInternally;
    bool m_isRequestedByPlugin;
    bool m_forceDownload;

    void doUpdatePlatformRequest() { }
    void doUpdateResourceRequest() { }

    PassOwnPtr<CrossThreadResourceRequestData> doPlatformCopyData(PassOwnPtr<CrossThreadResourceRequestData>) const;
    void doPlatformAdopt(PassOwnPtr<CrossThreadResourceRequestData>);
};

struct CrossThreadResourceRequestData : public CrossThreadResourceRequestDataBase {
    String m_token;
    String m_anchorText;
    bool m_isXMLHTTPRequest;
    bool m_mustHandleInternally;
    bool m_isRequestedByPlugin;
    bool m_forceDownload;
};

} // namespace WebCore

#endif // ResourceRequest_h
