using System;
using System.Collections.Generic;
using System.Text;

namespace StoryDesigner
{
    class UnexpectedValueException : ApplicationException
    {
        public UnexpectedValueException() { }

        public UnexpectedValueException(string message) : base(message) { }

        public UnexpectedValueException(string message, Exception inner) : base(message, inner) { }
    }
}
