#pragma once

namespace ss
{
	namespace util
	{
		inline namespace constraint
		{
			// Derive this so that the instance of the class can be moved but cannot be copied
			struct Only_Movable
			{
				Only_Movable() = default;
				Only_Movable(Only_Movable const&) = delete;
				Only_Movable(Only_Movable     &&) = default;
				Only_Movable & operator = (Only_Movable const&) = delete;
				Only_Movable & operator = (Only_Movable     &&) = default;
			};

			// Derive this so that the instance of the class can neither be moved nor be copied
			struct Non_Transferable
			{
				Non_Transferable() = default;
				Non_Transferable(Non_Transferable const&) = delete;
				Non_Transferable(Non_Transferable     &&) = delete;
				Non_Transferable & operator = (Non_Transferable const&) = delete;
				Non_Transferable & operator = (Non_Transferable     &&) = delete;
			};
		}
	}
}

