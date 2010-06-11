package org.ogre;


import java.io.IOException;
import java.io.InputStream;

import android.content.res.AssetManager;
import android.util.Log;

public class ACPManager {
	public static final int MAX_BUFFER = 1024*100;
	
	private AssetManager mAssets;
	
	private String mPrefix;
	
	private static ACPManager mInst = new ACPManager();
	
	private ACPManager(){
	}
	
	public static ACPManager getInstance(){
		return mInst;
	}
	
	public void setAssets(AssetManager assets, String prefix){
		mAssets = assets;
		mPrefix = prefix;
	}
	
	public boolean hasFile(String filename){
		filename = mPrefix + filename;
		
		boolean retval = false;
		InputStream is = null;
		try {
        	is = mAssets.open(filename); 
        	if(is != null){
        		retval = true;
        	}
        }
        catch (IOException ioe)
        {
        }
        finally
        {
            if (is != null)
            {
                try { is.close(); } catch (Exception e) {}
            }
        }
        return retval;
	}
	
	public int getFileSize(String filename){
		filename = mPrefix + filename;
		
		int size = 0;
		InputStream is = null;
		try {
        	is = mAssets.open(filename); 
            size = is.available();
        }
        catch (IOException ioe)
        {
        }
        finally
        {
            if (is != null)
            {
                try { is.close(); } catch (Exception e) {}
            }
        }
        return size;
	}
	
	/**
     * Helper class used to pass raw data around.  
     */
    public class RawData
    {
        /** The actual data bytes. */
        public byte[] data;
        /** The length of the data. */
        public int length;
    }
    
    private RawData mBuffer = null; // Internal instance to be reused
    private InputStream mStream = null; // Current open stream
    private String mFilename; // Current file being read
    
    public void beginStream(String filename) throws Exception
    {
    	filename = mPrefix + filename;
    	
    	if(mStream != null){
    		throw new Exception("beginStream called while another stream is still open");
    	}
    	
    	mFilename = filename;
        try {
        	mStream = mAssets.open(filename); 
            
        	if(mStream != null){
        		if(mBuffer == null){
        			mBuffer = new RawData();
        			mBuffer.data = new byte[MAX_BUFFER]; // Reusable 1mb buffer
        		}
        	}
        }
        catch (IOException ioe)
        {
        }
    }
    
    public void endStream()
    {
    	if(mStream != null)
    	{
    		try {
				mStream.close();
			} catch (IOException e) {
			}
    		mStream = null;
    	}
    }
    
    // Reads up to 1mb from the stream
    public RawData readStream(){
    	if(mStream == null){
    		return null;
    	}
    	
		try {
			int avail = mStream.available();
			if(avail > MAX_BUFFER){
				avail = MAX_BUFFER;
			}
			
			mBuffer.length = mStream.read(mBuffer.data, 0, avail); // Read data into buffer
		} catch (IOException e) {
		}
    	
    	return mBuffer;
    }
}
