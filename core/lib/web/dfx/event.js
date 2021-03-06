if (!window.dfx) {
    window.dfx = function() {};
    window.dfxjQuery = $.noConflict(true);
}

dfx.DOM_VK_DELETE    = 8;
dfx.DOM_VK_LEFT      = 37;
dfx.DOM_VK_UP        = 38;
dfx.DOM_VK_RIGHT     = 39;
dfx.DOM_VK_DOWN      = 40;
dfx.DOM_VK_ENTER     = 13;
dfx.DOM_VK_BACKSPACE = 46;

dfx.registeredEvents = null;


dfx.startMousePositionTrack = function(callback)
{
    /*
        //Example callback function.
        var setMousePos = function(e) {
            pageX = e.pageX;
            pageY = e.pageY;
        };
    */
    dfxjQuery(document.body).on('mousemove', null, null, callback);

};

dfx.stopMousePositionTrack = function(callback)
{
    dfxjQuery(document.body).off('mousemove', null, callback);

};

/**
 * Binds event type to specified elements.
 */
dfx.addEvent = function(elements, type, callback, data)
{
    if (elements) {
        if (type === 'safedblclick') {
            dfx.safedblclick(elements, callback, data);
        } else if (type === 'mousewheel') {
            dfxjQuery(elements).mousewheel(callback);
        } else {
            dfxjQuery(elements).on(type, null, data, callback);
        }
    }

};


dfx.safedblclick = function(elements, clickCallback, dblClickCallback, data)
{
    var t = null;
    dfxjQuery(elements).on('click', null, data, function(e) {
        clearTimeout(t);
        t = setTimeout(function() {
            clickCallback.call(this, e, data);
        }, 250);
    });

    dfxjQuery(elements).on('dblclick', null, data, function(e) {
        clearTimeout(t);
        dblClickCallback.call(this, e, data);
    });

};

/**
 * Trigger a type of event on all specified elements.
 */
dfx.trigger = function(elements, type, data)
{
    if (elements) {
        dfxjQuery(elements).trigger(type, data);
    }

};

/**
 * Removes bound event type from specified elements.
 */
dfx.removeEvent = function(elements, type, func)
{
    if (elements) {
        dfxjQuery(elements).off(type, null, null, func);
    }

};

dfx.hover = function(elements, over, out)
{
    if (elements) {
        dfxjQuery(elements).hover(over, out);
    }

};

/**
 * Toggle between two function calls.
 */
dfx.toggle = function(elements, fn, fn2)
{
    if (elements) {
        dfxjQuery(elements).toggle(fn, fn2);
    }

};


/**
 * Adds an onload event for the window.
 *
 * This is a shortcut for event.addEvent(window, 'load', func).
 *
 * @param funcPtr func Function to call when the page is loaded.
 *
 * @return bool
 */
dfx.addLoadEvent = function(func)
{
    dfxjQuery(document).ready(func);

};//end event.addLoadEvent()


/**
 * Shorthand for removing an event from an element and adding a new one.
 *
 * Removes the event from the element that calls func on eventType, and adds
 * a new one that calls newFunc on eventType.
 *
 * @param {DomElement}  element   The element to change the event on.
 * @param {String}      eventType The type of event to change (eg 'click').
 * @param {functionPtr} oldFunc   The function the event currently calls.
 * @param {functionPtr} newFunc   The function to change the event to call.
 *
 * @return void
 * @type   void
 */
dfx.changeEvent = function(element, eventType, oldFunc, newFunc)
{
    // Remove the old event.
    event.removeEvent(element, eventType, oldFunc);
    // Add the new one.
    event.addEvent(element, eventType, newFunc);

};//end event.changeEvent()


/**
 * Returns the position where a mouse event occured.
 *
 * When passed a valid mouse event, the return value will contain the x,y
 * coordinates of the position where the mouse event occured.
 *
 * @param Event evt The event object.
 *
 * @return Object
 */
dfx.getMouseEventPosition = function(evt)
{
    return {x: evt.pageX, y: evt.pageY};

};//end event.getMouseEventPosition()


/**
 * Returns the target that a mouse event occured on.
 *
 * @param Event evt The event object.
 *
 * @return The element that the event occured on.
 * @type   DomElement
 */
dfx.getMouseEventTarget = function(evt)
{
   var ret = null;
    if (evt.target) {
        ret = evt.target;
    } else if (evt.srcElement) {
        ret = evt.srcElement;
    }

    return ret;

};//end event.getMouseEventTarget()


/**
 * Prevents default action and event bubbling.
 *
 * @param Event evt The event object.
 */
dfx.preventDefault = function(e)
{
    e.preventDefault();

    // TODO: Remove stopPropagation call after fixing calling fns.
    dfx.stopPropagation(e);

};//end event.preventDefault()


dfx.stopPropagation = function(e)
{
    e.stopPropagation();

};

dfx.getEventType = function(e)
{
    return e.type;

};

dfx.which = function(e)
{
    return e.which;

};

dfx.getKeyChar = function(e)
{
    return String.fromCharCode(dfx.which(e));

};

dfx.resizeHeight = function(element, handle, startFn, endFn, moveFn, min, max)
{
    var elH   = dfx.getElementHeight(element);
    var mPosY = 0;
    min       = min || null;
    max       = max || null;

    if (dfx.isFn(moveFn) === false) {
        moveFn = function(){};
    }

    var move = function(e) {
        var pos = dfx.getMouseEventPosition(e);

        // Resize height.
        if (pos.y < mPosY) {
            elH = (elH + (mPosY - pos.y));
        } else if (pos.y > mPosY) {
            elH = (elH - (pos.y - mPosY));
        }

        if (elH >= 0 && (min === null || elH >= min) && (max === null || elH <= max)) {
            element.style.height = elH + 'px';
            var info = {
                prevPosY: mPosY,
                newPosY: pos.y,
                height: elH
            };

            moveFn.call(this, info);
        }

        mPosY = pos.y;
    };

    dfx.addEvent(handle, 'mousedown.drag', function(e) {
        elH   = dfx.getElementHeight(element);
        mPosY = 0;
        if (dfx.isFn(startFn) === true) {
            startFn.call(this);
        }

        mPosY = dfx.getMouseEventPosition(e).y;
        dfx.addEvent(document, 'mousemove.drag', function(e) {
            move(e);
        });

        dfx.addEvent(document, 'mouseup.drag', function() {
            // Remove the move event.
            dfx.removeEvent(document, 'mousemove.drag');
            dfx.removeEvent(document, 'mouseup.drag');
            if (dfx.isFn(endFn) === true) {
                endFn.call(this);
            }
        });
    });

};

dfx.drag = function(element, options, startFn, endFn, dragFn)
{
    var offset    = 0;
    var maxRight  = 0;
    var maxLeft   = 0;
    var elemWidth = dfx.getElementWidth(element);

    if (options) {
        if (options.maxLeft) {
            maxLeft = options.maxLeft;
        }
    }

    var drag = function(e) {
        var pos = (dfx.getMouseEventPosition(e).x + offset);
        if (pos < maxLeft) {
            pos = maxLeft;
        }

        if (maxRight !== 0 && pos > maxRight) {
            pos = maxRight;
        }

        if (pos >= 0) {
            // Process user supplied offset option, if any.
            var finalPos = pos;
            if (options) {
                if (options.offset) {
                    finalPos = (pos - options.offset);
                }
            }

            dfx.setStyle(element, 'left', finalPos + 'px');

            if (dragFn) {
                dragFn.call(this, finalPos, maxRight);
            }
        }
    };

    dfx.addEvent(element, 'mousedown.drag', function(e) {
        if (dfx.isFn(startFn) === true) {
            startFn.call(this);
        }

        var elemX = dfx.getElementCoords(element).x;
        var mPosX = dfx.getMouseEventPosition(e).x;
        offset    = (elemX - mPosX);

        // Get the max right.
        maxRight = (dfx.getWindowDimensions().width - elemWidth);
        if (options) {
            if (options.maxRight) {
                maxRight = options.maxRight;
            }
        }

        dfx.addEvent(document, 'mousemove.drag', function(e) {
            drag(e);
        });

        dfx.addEvent(document, 'mouseup.drag', function() {
            // Remove the move event.
            dfx.removeEvent(document, 'mousemove.drag');
            dfx.removeEvent(document, 'mouseup.drag');
            if (dfx.isFn(endFn) === true) {
                endFn.call(this, maxRight);
            }
        });
    });

};
