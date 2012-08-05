
class Test2{
	
	private static void throwException() throws IllegalArgumentException{
		throw new IllegalArgumentException("I know exceptions");
	}
	
	private static void testException(){
		try{
			throwException();
		}
		catch(Exception e){
			e.printStackTrace();
		}
	}
	
	public static void main(String [] args){
		int a = 4;
		int b = 5;
		System.out.println("Hello World! "+a+"+"+b+"="+(a+b));
		testException();
	};
};
