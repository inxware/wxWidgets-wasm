/////////////////////////////////////////////////////////////////////////////
// Name:        webrequest.h
// Created:     2018-10-14
// Copyright:   (c) 2018 wxWidgets development team
// Licence:     wxWindows licence
/////////////////////////////////////////////////////////////////////////////

/**
    @class wxWebRequest

    This class allows for simple HTTP requests using the operating system's
    components as implementation.

    The latest features of the operating system will be used if available
    (e.g. HTTP/2, TLS 1.3).
    System-wide configuration like proxy and SSL certificates will be used
    when possible.

    Instances of wxWebRequest are created by using
    wxWebSession::CreateRequest().

    The requests are handled asynchronously and event handlers are used to
    communicate the request status. The response data may be stored in
    memory, to a file or processed directly, see SetStorage() for details.

    Example usage in an event handler function of some window (i.e. @c this in
    the example below is a wxWindow pointer):

    @code
        // Create the request object
        wxWebRequest request = wxWebSession::GetDefault().CreateRequest(
            this,
            "https://www.wxwidgets.org/downloads/logos/blocks.png"
        );

        if ( !request.IsOk() ) {
            // This is not expected, but handle the error somehow.
        }

        // Bind state event
        Bind(wxEVT_WEBREQUEST_STATE, [](wxWebRequestEvent& evt) {
            switch (evt.GetState())
            {
                // Request completed
                case wxWebRequest::State_Completed:
                {
                    wxImage logoImage(*evt.GetResponse().GetStream());
                    if (logoImage.IsOK())
                        wxLogInfo("Image loaded");
                    break;
                }
                // Request failed
                case wxWebRequest::State_Failed:
                    wxLogError("Could not load logo: %s", evt.GetErrorDescription());
                    break;
            }

        });

        // Start the request
        request.Start();
    @endcode

    @section descriptions Implementation Descriptions

    The following APIs are used per platform, additional details
    about supported features may be found in their documentation.

    Available features by implementation and minimum version:
    <table>
    <tr><th>Operating System</th><th>API</th><th>HTTPS</th><th>HTTP/2</th></tr>
    <tr>
        <td>Windows</td>
        <td>
            <a target=_new href="https://docs.microsoft.com/en-us/windows/desktop/WinHttp/about-winhttp">WinHTTP</a>
        </td>
        <td>Yes</td>
        <td>Windows 10 1607</td>
    </tr>
    <tr>
        <td>macOS</td>
        <td>
            <a target=_new href="https://developer.apple.com/documentation/foundation/urlsession">NSURLSession</a>
        </td>
        <td>macOS 10.9</td>
        <td>macOS 10.11</td>
    </tr>
    <tr>
        <td>iOS</td>
        <td>
            <a target=_new href="https://developer.apple.com/documentation/foundation/urlsession">NSURLSession</a>
        </td>
        <td>iOS 7.0</td>
        <td>iOS 9.0</td>
    </tr>
    <tr>
        <td>Linux</td>
        <td>
            <a target=_new href="https://curl.haxx.se/libcurl/">libcurl</a>
        </td>
        <td>Yes</td>
        <td>7.47.0</td>
    </tr>
    </table>

    @beginEventEmissionTable{wxWebRequestEvent}
    @event{wxEVT_WEBREQUEST_STATE(id, func)}
        The request state changed.
    @event{wxEVT_WEBREQUEST_DATA(id, func)}
        A new block of data has been downloaded.
    @endEventTable

    @since 3.1.5

    @library{wxnet}
    @category{net}

    @see wxWebResponse, wxWebSession
*/
class wxWebRequest
{
public:
    /**
        Possible request states returned by GetState().
    */
    enum State
    {
        /// The request has just been created and Start() has not been called
        State_Idle,

        /**
            The request is currently unauthorized. Calling GetAuthChallenge()
            returns a challenge object with further details.
        */
        State_Unauthorized,

        /// The request has been started and is transferring data
        State_Active,

        /// The request completed successfully and all data has been received.
        State_Completed,

        /// The request failed
        State_Failed,

        /// The request has been cancelled before completion by calling Cancel()
        State_Cancelled
    };

    /**
        Possible storage types. Set by SetStorage().
    */
    enum Storage
    {
        /// All data is collected in memory until the request is complete
        Storage_Memory,

        /// The data is written to a file on disk
        Storage_File,

        /**
            The data is not stored by the request and is only available
            via events.
        */
        Storage_None
    };

    /**
        Default constructor creates an invalid object.

        Initialize it by assigning wxWebSession::CreateRequest() to it before
        using it.

        @see IsOk()
    */
    wxWebRequest();

    /**
        Check if the object is valid.

        No other methods can be used if this function returns @false.
    */
    bool IsOk() const;

    /**
        Send the request to the server asynchronously.

        Events will be triggered on success or failure.

        The current state must be @c State_Idle, already started requests can't
        be started again.

        @see Cancel()
    */
    void Start();

    /**
        Cancel an active request.
    */
    void Cancel();

    /**
        Returns a response object after a successful request.

        Before sending a request or after a failed request this will return
        an invalid response object, i.e. such that wxWebResponse::IsOk()
        returns @NULL.
    */
    wxWebResponse GetResponse() const;

    /**
        Returns the current authentication challenge object while the request
        is in @c State_Unauthorized.
    */
    wxWebAuthChallenge GetAuthChallenge() const;

    /**
        Returns the id specified while creating this request.
    */
    int GetId() const;

    /** @name Request options
        Methods that set options before starting the request
    */
    ///@{
    /**
        Sets a request header which will be sent to the server by this request.

        The header will be added if it hasn't been set before or replaced
        otherwise.

        @param name
            Name of the header
        @param value
            String value of the header. An empty string will remove the header.
    */
    void SetHeader(const wxString& name, const wxString& value);

    /**
        Set <a href="http://www.w3.org/Protocols/rfc2616/rfc2616-sec9.html">common</a>
        or expanded HTTP method.

        The default method is GET unless request data is provided in which
        case POST is the default.

        @param method
            HTTP method name, e.g. "GET".
    */
    void SetMethod(const wxString& method);

    /**
        Set the text to be posted to the server.

        After a successful call to this method, the request will use HTTP @c
        POST instead of the default @c GET when it's executed.

        @param text
            The text data to post.
        @param contentType
            The value of HTTP "Content-Type" header, e.g. "text/html;
            charset=UTF-8".
        @param conv
            Conversion used when sending the text to the server
    */
    void SetData(const wxString& text, const wxString& contentType,
        const wxMBConv& conv = wxConvUTF8);

    /**
        Set the binary data to be posted to the server.

        The next request will be a HTTP @c POST instead of the default HTTP
        @c GET and the given @a dataStream will be posted as the body of
        this request.

        Example of use:
        @code
        std::unique_ptr<wxInputStream> stream(new wxFileInputStream("some_file.dat"));
        if ( !stream->IsOk() ) {
            // Handle error (due to e.g. file not found) here.
            ...
            return;
        }
        request.SetData(stream.release(), "application/octet-stream")
        @endcode

        @param dataStream
            The data in this stream will be posted as the request body. The
            pointer may be @NULL, which will result in sending 0 bytes of data,
            but if not empty, should be valid, i.e. wxInputStream::IsOk() must
            return @true. This object takes ownership of the passed in pointer
            and will delete it, i.e. the pointer must be heap-allocated.
        @param contentType
            The value of HTTP "Content-Type" header, e.g.
            "application/octet-stream".
        @param dataSize
            Amount of data which is sent to the server. If set to
            @c wxInvalidOffset all stream data is sent.

        @return @false if @a dataStream is not-empty but invalid or if @a
            dataSize is not specified and the attempt to determine stream size
            failed; @true in all the other cases.
    */
    bool SetData(wxInputStream* dataStream,
        const wxString& contentType, wxFileOffset dataSize = wxInvalidOffset);

    /**
        Instructs the request to ignore server error status codes.

        Per default, server side errors (status code 400-599) will enter
        the State_Failed state just like network errors, but
        if the response is still required in such cases (e.g. to get more
        details from the response body), set this option to ignore all errors.
        If ignored, wxWebResponse::GetStatus() has to be checked
        from the State_Completed event handler.
    */
    void SetIgnoreServerErrorStatus(bool ignore);

    /**
        Sets how response data will be stored.

        The default storage method @c Storage_Memory collects all response data
        in memory until the request is completed. This is fine for most usage
        scenarios like API calls, loading images, etc. For larger downloads or
        if the response data will be used permanently @c Storage_File instructs
        the request to write the response to a temporary file. This temporary
        file may then be read or moved after the request is complete. The file
        will be downloaded to the system temp directory as returned by
        wxStandardPaths::GetTempDir(). To specify a different directory use
        wxWebSession::SetTempDir().

        Sometimes response data needs to be processed while its downloaded from
        the server. For example if the response is in a format that can be
        parsed piece by piece like XML, JSON or an archive format like ZIP.
        In these cases storing the data in memory or a file before being able
        to process it might not be ideal and @c Storage_None should be set.
        With this storage method the data is only available during the
        @c wxEVT_WEBREQUEST_DATA event calls as soon as it's received from the
        server.
    */
    void SetStorage(Storage storage);
    ///@}

    /** @name Progress methods
        Methods that describe the requests progress
    */
    ///@{
    /**
        Returns the current state of the request.
    */
    State GetState() const;

    /// Returns the number of bytes sent to the server.
    wxFileOffset GetBytesSent() const;

    /// Returns the number of bytes expected to be sent to the server.
    wxFileOffset GetBytesExpectedToSend() const;

    /// Returns the number of bytes received from the server.
    wxFileOffset GetBytesReceived() const;

    /**
        Returns the number of bytes expected to be received from the server.

        This value is based on the @c Content-Length header, if none is found
        it will return -1.
    */
    wxFileOffset GetBytesExpectedToReceive() const;
    ///@}
};

/**
    Authentication challenge information available via
    wxWebRequest::GetAuthChallenge().

    Use SetCredentials() to provide user credentials.
*/
class wxWebAuthChallenge
{
public:
    enum Source
    {
        /// The server requested authentication
        Source_Server,

        /// A proxy requested authentication
        Source_Proxy
    };

    /**
        Returns which source requested credentials with this challenge.
    */
    Source GetSource() const;

    /**
        Used to provide user credentials to the authentication challenge.

        @param user
            User name.
        @param password
            The users password.
    */
    void SetCredentials(const wxString& user, const wxString& password);
};

/**
    A wxWebResponse allows access to the response sent by the server.

    @since 3.1.5

    @library{wxnet}
    @category{net}

    @see wxWebRequest
*/
class wxWebResponse
{
public:
    /**
        Default constructor creates an invalid object.

        Initialize it by assigning wxWebRequest::GetResponse() to it before
        using it.

        @see IsOk()
    */
    wxWebResponse();

    /**
        Check if the object is valid.

        No other methods can be used if this function returns @false.
    */
    bool IsOk() const;

    /**
        Returns the final URL.
        This URL might be different than the request URL when a redirection
        occurred.
    */
    wxString GetURL() const;

    /**
        Returns a header from the response or an empty string if the header
        could not be found.

        @param name Name of the header field
    */
    wxString GetHeader(const wxString& name) const;

    /**
        Returns the MIME type of the response (if available).
    */
    wxString GetMimeType() const;

    /**
        Returns the status code returned by the server.
    */
    int GetStatus() const;

    /**
        Returns the status text of the response.
    */
    wxString GetStatusText() const;

    /**
        Returns a stream which represents the response data sent by the server.
    */
    wxInputStream* GetStream();

    /**
        Returns a suggested filename for the response data.
    */
    wxString GetSuggestedFileName() const;

    /**
        Returns all response data as a string.

        This is mostly useful for debugging or diagnostics.
    */
    wxString AsString() const;
};

/**
    @class wxWebSession

    Session allows creating wxWebRequest objects used for the actual HTTP
    requests.

    It also handles session-wide parameters and data used by wxWebRequest
    instances.

    Usually the default session available via wxWebSession::GetDefault()
    should be used. Additional instances might be useful if session separation
    is required. Instances <b>must not</b> be deleted before every active web
    request has finished.

    Every wxWebRequest sharing the same session object will use the same
    cookies. Additionally, an underlying network connection might be kept
    alive to achieve faster additional responses.

    @since 3.1.5

    @library{wxnet}
    @category{net}

    @see wxWebRequest
*/
class wxWebSession
{
public:
    /**
        Create a new request for the specified URL.

        The specified objects will be notified via wxWebRequestEvent objects
        when the request state changes, e.g. when it is completed. It must be
        specified and its lifetime must be long enough to last until the
        request is completed. In particular, if the handler is a top-level
        window, the request must be cancelled before the window can be closed
        and destroyed.

        @param handler
            The handler object to notify, must be non-@NULL.
        @param url
            The URL of the HTTP resource for this request
        @param id
            Optional id sent with events
        @return
            The new request object, use wxWebRequest::IsOk() to check if its
            creation has succeeded.
    */
    wxWebRequest
    CreateRequest(wxEvtHandler* handler, const wxString& url, int id = wxID_ANY);

    /**
        Retrieve the version information about the implementation library used
        by this session.
    */
    virtual wxVersionInfo GetLibraryVersionInfo();

    /**
        Sets a request header in every wxWebRequest created from this session
        after is has been set.

        A good example for a session-wide request header is the @c User-Agent
        header.

        @param name Name of the header
        @param value String value of the header
    */
    void SetHeader(const wxString& name, const wxString& value);

    /**
        Override the default temporary directory that may be used by the
        session implementation, when required.
    */
    void SetTempDir(const wxString& dir);

    /**
        Returns the current temporary directory.

        @see SetTempDir()
    */
    wxString GetTempDir() const;

    /**
        Returns the default session
    */
    static wxWebSession& GetDefault();

    /**
        Creates a new wxWebSession object.

        Use IsOpened() to check if the session creation succeeded.

        @param backend
            The backend web session implementation to use.

        @return
            The created wxWebSession
    */
    static wxWebSession New(const wxString& backend = wxWebSessionBackendDefault);

    /**
        Allows to check if the specified backend is available at runtime.

        Usually the default backend should always be available, but e.g. macOS
        before 10.9 does not have the @c NSURLSession implementation available.
    */
    static bool IsBackendAvailable(const wxString& backend);

    /**
        Return true if the session was successfully opened and can be used.
    */
    bool IsOpened() const;

    /**
        Close the session.

        This frees any resources associated with the session and puts it in an
        invalid state. Another session object can be assigned to it later to
        allow using this object again.
     */
    void Close();
};

/**
    @class wxWebRequestEvent

    A web request event sent during or after server communication.

    @since 3.1.5

    @library{wxnet}
    @category{net}

    @see wxWebRequest
*/
class wxWebRequestEvent : public wxEvent
{
public:
    /**
        Return the current state of the request
    */
    wxWebRequest::State GetState() const;

    /**
        The response with the state set to @c State_Complete or empty response
        object for other events.
    */
    const wxWebResponse& GetResponse() const;

    /**
        A textual error description for a client side error
        in case of @c State_Failed
    */
    const wxString& GetErrorDescription() const;

    /**
        Returns a file name to a temporary file containing the response data
        when the state is @c State_Completed and storage is @Storage_File.

        The file will be removed after the event handlers are called. You can
        move the file to a location of your choice if you want to process the
        contents outside the event handler.
    */
    const wxString& GetResponseFileName() const;

    /**
        Only for @c wxEVT_WEBREQUEST_DATA events. The buffer is only valid
        inside the event handler.
    */
    ///@{
    const void* GetDataBuffer() const;

    size_t GetDataSize() const;
    ///@}
};

wxEventType wxEVT_WEBREQUEST_STATE;
wxEventType wxEVT_WEBREQUEST_DATA;
