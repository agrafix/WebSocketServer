-- Echo
function main (clientId, message)
    -- echo back the message
    _send(clientId, message)
end

-- notify about connects
function on_connect(clientId)
    _sendAll("ClientID " .. clientId .. " just joined the server")
end

-- ... and disconnects
function on_disconnect(clientId)
    _sendAll("ClientID " .. clientId .. " just quit the server")
end
